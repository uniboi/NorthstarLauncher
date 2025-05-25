#include "engine/datamap.h"

#include "tier0/filestream.h"
#include "engine/host.h"

#include <sstream>
#include <cstdarg>

const char* DataMap_UndefinedBufferTypeStr(int size)
{
	// just leak it idc
	char* buf = (char*)malloc(sizeof("undefined[]") + 3);
	memset(buf, 0, sizeof("undefined[]") + 3);
	sprintf(buf, "[%d]_undefined", size);
	return buf;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string from fieldtype_t enum entry
// Input  : type -
//-----------------------------------------------------------------------------
const char* DataMap_GetFieldTypeStr(fieldtype_t type, int size)
{
	switch (type)
	{
	case FIELD_VOID:
	{
		if (size == 8)
		{
			return "*anyopaque";
		}

		if (size != 0)
		{
			return DataMap_UndefinedBufferTypeStr(size);
		}
		return "void";
	}
	case FIELD_FLOAT:
		return "f32";
	case FIELD_STRING:
		return "[*:0]u8";
	case FIELD_VECTOR:
		return "Vector3";
	case FIELD_QUATERNION:
		return "Quaternion";
	case FIELD_INTEGER:
		// return "i32";
		return DataMap_GetSizeTypeStr(size);
	case FIELD_BOOLEAN:
		return "bool";
	case FIELD_SHORT:
		return "i16";
	case FIELD_CHARACTER:
		return "u8";
	case FIELD_COLOR32:
		return "Color32";
	case FIELD_EMBEDDED:
		return "embedded";
	case FIELD_CUSTOM:
		return DataMap_UndefinedBufferTypeStr(size);
	case FIELD_CLASSPTR:
		return "*CBaseEntity";
	case FIELD_EHANDLE:
		if (size == 5)
			return "EHANDLE5"; // SmartAmmo_WeaponData has a couple 5 byte ehandles for some reason???
		return "EHANDLE";
	case FIELD_EDICT:
		return "*edict_t";
	case FIELD_POSITION_VECTOR:
		return "Vector3";
	case FIELD_TIME:
		return "time";
	case FIELD_TICK:
		return "tick";
	case FIELD_MODELNAME:
		return "ModelName";
	case FIELD_SOUNDNAME:
		return "soundName";
	case FIELD_INPUT:
		return "Input";
	case FIELD_FUNCTION:
		return "Func";
	case FIELD_VMATRIX:
		return "Matrix";
	case FIELD_VMATRIX_WORLDSPACE:
		return "MatrixWrld";
	case FIELD_MATRIX3X4_WORLDSPACE:
		return "Matrix3x4";
	case FIELD_INTERVAL:
		return "Interval";
	case FIELD_MODELINDEX:
		return "ModelIndex";
	case FIELD_MATERIALINDEX:
		return "MatIndex";
	case FIELD_VECTOR2D:
		return "Vector2";
	case FIELD_INTEGER64:
		return "i64";
	case FIELD_VECTOR4D:
		return "Vector4";
	}

	if (size == 0x8)
	{
		return "*anyopaque";
	}

	return DataMap_GetSizeTypeStr(size);
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string based on number of bytes
// Input  : iBytes -
//-----------------------------------------------------------------------------
const char* DataMap_GetSizeTypeStr(int iBytes)
{
	switch (iBytes)
	{
	case 1:
		return "u8";
	case 2:
		return "i16";
	case 4:
		return "i32";
	case 8:
		return "i64";
	case 16:
		return "i128";
	}

	return DataMap_UndefinedBufferTypeStr(iBytes);
}

bool DataMap_HasVTable(datamap_t* pMap)
{
	datamap_t* pCurMap = pMap;
	while (pCurMap->pBaseMap)
	{
		pCurMap = pCurMap->pBaseMap;
	}
	if (pCurMap->dataNumFields == 0)
		return false;
	return pCurMap->dataDesc[0].fieldOffset == 0x8;
}

typedescription_t* DataMap_NextPhysicalField(datamap_t* pMap, int idx)
{
	int nFields = pMap->dataNumFields;
	if (idx + 1 == nFields)
	{
		return nullptr;
	}

	typedescription_t* pSelf = &pMap->dataDesc[idx];
	for (int i = idx + 1; i < nFields; i++)
	{
		typedescription_t* pCur = &pMap->dataDesc[i];
		if (pCur->fieldOffset != 0)
		{
			return pCur;
		}
	}

	return nullptr;
}

typedescription_t* DataMap_PreviousField(datamap_t* pMap, int idx)
{
	if (idx == 0)
		return nullptr;

	for (int i = idx - 1; i > 0; i--)
	{
		typedescription_t* pCur = &pMap->dataDesc[i];
		if (pCur->fieldOffset != 0)
		{
			return pCur;
		}
	}

	return &pMap->dataDesc[0];
}

int DataMap_Alignment(datamap_t* pMap)
{
	// TODO flesh this out if needed
	int nFields = pMap->dataNumFields;

	// check for vtable pointer
	if (DataMap_HasVTable(pMap))
	{
		return 8;
	}

	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurField = &pMap->dataDesc[i];
		if (pCurField->fieldSizeInBytes >= 8)
			return 8;
	}

	return 4;
}

typedescription_t* DataMap_DumpFields(datamap_t* pMap, std::string& svBuffer)
{
	if (DataMap_HasVTable(pMap))
		svBuffer += "  vtable: *anyopaque,\n";

	if (pMap->pBaseMap)
	{
		// include the base class without any vtable members
		svBuffer += FormatA("  %s: Inherit(%s),\n\n", pMap->pBaseMap->dataClassName, pMap->pBaseMap->dataClassName);
	}

	typedescription_t* lastPhysicalField = nullptr;

	int nFields = pMap->dataNumFields;
	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		if ((i > 0 && pCurDesc->fieldOffset == 0))
			continue;
		if (lastPhysicalField && pCurDesc->fieldOffset < lastPhysicalField->fieldOffset + lastPhysicalField->fieldSizeInBytes)
			continue;
		if (pCurDesc->fieldSizeInBytes == 0)
			continue;

		lastPhysicalField = pCurDesc;

		// padding
		if (typedescription_t* pLast = DataMap_PreviousField(pMap, i); pLast && pLast->fieldOffset + pLast->fieldSizeInBytes < pCurDesc->fieldOffset)
		{
			int nPadBytes = pCurDesc->fieldOffset - (pLast->fieldOffset + pLast->fieldSizeInBytes);

			// sometimes the calculation of field size is incorrect in the datamap (??) leading to huge gaps that are incorrect.
			// padding larger than the structure alignment wouldn't be inserted by the compiler hinting to a pad size calculation error.
			// if (nPadBytes <= DataMap_Alignment(pMap))
			if (nPadBytes <= 8)
			{
				svBuffer += FormatA("    gap_%x: [%i]_undefined,\n", pCurDesc->fieldOffset - nPadBytes, nPadBytes);
			}
		}

		svBuffer += FormatA("    %s: ", pCurDesc->fieldName); // Print name

		// field type
		if (pCurDesc->fieldSize != 1)
		{
			svBuffer += FormatA("[%d]", pCurDesc->fieldSize); // Print array size
		}
		if (pCurDesc->td) // Print Type
		{
			svBuffer += FormatA("%s", pCurDesc->td->dataClassName);
		}
		else
		{
			svBuffer += FormatA("%s", DataMap_GetFieldTypeStr(pCurDesc->fieldType, pCurDesc->fieldSizeInBytes / pCurDesc->fieldSize));
		}

		svBuffer += FormatA(", // +0x%x size: 0x%x (0x%x * 0x%x) %i\n", pCurDesc->fieldOffset, pCurDesc->fieldSizeInBytes, pCurDesc->fieldSize, pCurDesc->fieldSizeInBytes / pCurDesc->fieldSize, pCurDesc->fieldType);
	}

	return lastPhysicalField;
}

int DataMap_TotalSize(int lastFieldOffset, int lastFieldSize, int alignment)
{
	if ((lastFieldOffset + lastFieldSize) % alignment == 0)
		return lastFieldOffset + lastFieldSize;
	return lastFieldOffset + (alignment - ((lastFieldOffset + lastFieldSize) % alignment)) + lastFieldSize;
}

void DataMap_DumpEmbeddedMaps(datamap_t*, std::string&, std::unordered_set<std::string>&);

//-----------------------------------------------------------------------------
// Purpose: Dumps a datamap_t pointer to a std::string passed by ref
// Input  : *pMap -
//          &svBuffer -
//-----------------------------------------------------------------------------
void DataMap_DumpStr(datamap_t* pMap, std::string& svBuffer, std::unordered_set<std::string>& structs)
{
	if (structs.count(pMap->dataClassName) == 0)
	{
		svBuffer += FormatA("pub const %s = extern struct {\n", pMap->dataClassName);
		typedescription_t* lastField = DataMap_DumpFields(pMap, svBuffer);
		svBuffer += "\n    test {\n";

		int alignment = DataMap_Alignment(pMap);
		int totalSize = DataMap_TotalSize(lastField->fieldOffset, lastField->fieldSizeInBytes, alignment);
		svBuffer += FormatA("        try std.testing.expect(@sizeOf(@This()) == 0x%x);\n", totalSize);

		int nFields = pMap->dataNumFields;
		typedescription_t* lastPhysicalField = nullptr;
		for (int i = 0; i < nFields; i++)
		{
			typedescription_t* pCurDesc = &pMap->dataDesc[i];

			if ((i > 0 && pCurDesc->fieldOffset == 0))
				continue;
			if (lastPhysicalField && pCurDesc->fieldOffset < lastPhysicalField->fieldOffset + lastPhysicalField->fieldSizeInBytes)
				continue;
			if (pCurDesc->fieldSizeInBytes == 0)
				continue;

			lastPhysicalField = pCurDesc;

			svBuffer += FormatA("        try std.testing.expect(@offsetOf(@This(), \"%s\") == 0x%x);\n", pCurDesc->fieldName, pCurDesc->fieldOffset);
		}

		svBuffer += "    }\n";
		svBuffer += "};\n\n";

		structs.insert(pMap->dataClassName);
	}

	DataMap_DumpEmbeddedMaps(pMap, svBuffer, structs);
}

void DataMap_DumpEmbeddedMaps(datamap_t* pMap, std::string& svBuffer, std::unordered_set<std::string>& structs)
{
	int nFields = pMap->dataNumFields;
	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		if (pCurDesc->td && structs.count(pCurDesc->td->dataClassName) == 0)
		{
			DataMap_DumpStr(pCurDesc->td, svBuffer, structs);
		}
	}

	if (pMap->pBaseMap && structs.count(pMap->pBaseMap->dataClassName) == 0)
	{
		DataMap_DumpStr(pMap->pBaseMap, svBuffer, structs);
		DataMap_DumpEmbeddedMaps(pMap->pBaseMap, svBuffer, structs);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Dumps a datamap_t pointer to disk
// Input  : *pMap -
//-----------------------------------------------------------------------------
void DataMap_Dump(datamap_t* pMap)
{
	if (!pMap)
	{
		Error(eLog::ENGINE, NO_ERROR, "Datamap is null!");
		return;
	}

	DevMsg(eLog::NS, "Dumping datamap\n");

	std::string svAddTypes; // Additional types used in this map
	std::string svMapFile; // This map

	std::unordered_set<std::string> structs = {};

	// dump our map
	DataMap_DumpStr(pMap, svMapFile, structs);

	// Write it to disk
	CFileStream fStream;
	fStream.Open(FormatA("%s\\Map_%s.txt", g_pEngineParms->szModName, pMap->dataClassName).c_str(), CFileStream::WRITE);
	fStream.WriteString(svAddTypes);
	fStream.WriteString(svMapFile);
	fStream.Close();

	DevMsg(eLog::NS, "Succesfully dumped %s\n", pMap->dataClassName);

	// if (pMap->pBaseMap)
	//{
	//	DataMap_Dump(pMap->pBaseMap);
	// }
}
