#include "engine/datamap.h"

#include "tier0/filestream.h"
#include "engine/host.h"

#include <sstream>
#include <cstdarg>

char staticTempBuffer[1024];

const char* DataMap_UndefinedBufferTypeStr(int size)
{
	// just leak it idc
	// char* buf = (char*)malloc(sizeof("undefined[]") + 6);
	// memset(buf, 0, sizeof("undefined[]") + 6);
	sprintf(staticTempBuffer, "[%d]valve.@\"undefined\"", size);
	return staticTempBuffer;
}

//-----------------------------------------------------------------------------
// Purpose: Returns a string from fieldtype_t enum entry
// Input  : type -
//-----------------------------------------------------------------------------
const char* DataMap_GetFieldTypeStr(typedescription_t* desc)
{
	int size = desc->fieldSizeInBytes / MAX(desc->fieldSize, 1);

	switch (desc->fieldType)
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
		return "m.Vector3";
	case FIELD_QUATERNION:
		return "m.Quaternion";
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
		return "valve.Color32";
	case FIELD_EMBEDDED:
	case FIELD_CUSTOM:
		// case FIELD_UNKNOWN:
		return DataMap_UndefinedBufferTypeStr(size);
	case FIELD_CLASSPTR:
		return "*server.CBaseEntity";
	case FIELD_EHANDLE:
	{
		// don't use calculated type size bc respawn fucked up for some ehandle arrays
		// if (desc->fieldSizeInBytes == 4)
		//	return "valve.EHANDLE";

		if (desc->fieldSize == 1 && desc->fieldSizeInBytes > 4)
		{
			sprintf(staticTempBuffer, "[%d]valve.EHANDLE", desc->fieldSizeInBytes / 4);
			return staticTempBuffer;
		}

		return "valve.EHANDLE";
	}
	case FIELD_EDICT:
		return "valve.edict_t";
	case FIELD_POSITION_VECTOR:
		return "m.Vector3";
	case FIELD_TIME:
		return "valve.time";
	case FIELD_TICK:
		return "valve.tick";
	case FIELD_MODELNAME:
		return "valve.ModelName";
	case FIELD_SOUNDNAME:
		return "valve.SoundName";
	case FIELD_INPUT:
		return "valve.Input";
	case FIELD_FUNCTION:
		return "valve.Func";
	case FIELD_VMATRIX:
		return "valve.Matrix";
	case FIELD_VMATRIX_WORLDSPACE:
		return "valve.MatrixWrld";
	case FIELD_MATRIX3X4_WORLDSPACE:
		return "valve.Matrix3x4";
	case FIELD_INTERVAL:
		return "valve.Interval";
	case FIELD_MODELINDEX:
		return "valve.ModelIndex";
	case FIELD_MATERIALINDEX:
		return "valve.MatIndex";
	case FIELD_VECTOR2D:
		return "m.Vector2";
	case FIELD_INTEGER64:
		return "i64";
	case FIELD_VECTOR4D:
		return "m.Vector4";
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

typedescription_t* DataMap_LastPhysicalField(datamap_t* pMap)
{
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
	}
	return lastPhysicalField;
}

const char* TypeDesc_FieldName(const char* name)
{
	memset(staticTempBuffer, 0, 1024);
	strcpy(staticTempBuffer, name);

	for (int i = 0; i < sizeof(staticTempBuffer); i++)
	{
		char c = staticTempBuffer[i];
		if (c == 0)
			break;
		if (c == '[' || c == ']' || c == '.')
			staticTempBuffer[i] = '_';
	}

	return staticTempBuffer;
}

int TotalFieldSize(typedescription_t* field)
{
	switch (field->fieldType)
	{
	case FIELD_EMBEDDED:
		return field->fieldSizeInBytes * field->fieldSize;
	default:
		return field->fieldSizeInBytes;
	}
}

int DataMap_TotalSize(datamap_t* pMap, int alignment)
{
	if (!pMap)
		return 0;

	//// They fatfingered the type size of m_playerObserver
	// if (strcmp(pMap->dataClassName, "CGlobalNonRewinding") == 0)
	//{
	//	return 0xbe0;
	// }

	typedescription_t* lastField = DataMap_LastPhysicalField(pMap);

	if (!lastField)
		return DataMap_TotalSize(pMap->pBaseMap, alignment);

	// if ((lastField->fieldOffset + lastField->fieldSizeInBytes) % alignment == 0)
	//	return lastField->fieldOffset + lastField->fieldSizeInBytes;
	// return lastField->fieldOffset + (alignment - ((lastField->fieldOffset + lastField->fieldSizeInBytes) % alignment)) + lastField->fieldSizeInBytes;

	if ((lastField->fieldOffset + TotalFieldSize(lastField)) % alignment == 0)
		return lastField->fieldOffset + TotalFieldSize(lastField);
	return lastField->fieldOffset + (alignment - ((lastField->fieldOffset + TotalFieldSize(lastField)) % alignment)) + TotalFieldSize(lastField);
}

typedescription_t* DataMap_DumpFields(datamap_t* pMap, CFileStream& fstream)
{
	if (DataMap_HasVTable(pMap))
	{
		fstream.WriteString("    vtable: *anyopaque,\n");
	}

	if (pMap->pBaseMap)
	{
		// include the base class without any vtable members
		fstream.WriteString(FormatA("    %s: abi.Inherit(server.%s),\n\n", pMap->pBaseMap->dataClassName, pMap->pBaseMap->dataClassName));
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

		// padding
		// if (typedescription_t* pLast = DataMap_PreviousField(pMap, i); pLast && pLast->fieldOffset + pLast->fieldSizeInBytes < pCurDesc->fieldOffset)
		if (lastPhysicalField && lastPhysicalField->fieldOffset + lastPhysicalField->fieldSizeInBytes < pCurDesc->fieldOffset)
		{
			int nPadBytes = pCurDesc->fieldOffset - (lastPhysicalField->fieldOffset + lastPhysicalField->fieldSizeInBytes);

			// sometimes the field size is incorrect in the datamap (??) leading to huge gaps that are incorrect.
			// padding larger than the structure alignment wouldn't be inserted by the compiler hinting to a pad size calculation error.
			// if (nPadBytes <= DataMap_Alignment(pMap))
			if (nPadBytes <= 8)
			{
				fstream.WriteString(FormatA("    gap_%x: [%i]valve.@\"undefined\",\n", pCurDesc->fieldOffset - nPadBytes, nPadBytes));
			}
		}

		lastPhysicalField = pCurDesc;

		fstream.WriteString(FormatA("    %s: ", TypeDesc_FieldName(pCurDesc->fieldName)));

		// if (false && pCurDesc->fieldSizeInBytes % MAX(pCurDesc->fieldSize, 1))
		//{
		//	// they messed up either fieldSizeinBytes or fieldSize
		//	fstream.WriteString(DataMap_UndefinedBufferTypeStr(pCurDesc->fieldSizeInBytes));
		//	fstream.WriteString(", // bruh ");
		// }
		// else
		{
			int undividable = pCurDesc->fieldSizeInBytes % MAX(pCurDesc->fieldSize, 1);
			if (undividable && pCurDesc->fieldType == FIELD_UNKNOWN)
			{
				fstream.WriteString(DataMap_UndefinedBufferTypeStr(pCurDesc->fieldSizeInBytes));
			}
			else
			{
				if (pCurDesc->fieldSize > 1)
				{
					int fieldSize = pCurDesc->fieldSize;

					// ehandles are always 4 byte.
					// fieldSize is sometimes incorrect.
					if (pCurDesc->fieldType == FIELD_EHANDLE && (pCurDesc->fieldSizeInBytes / MAX(pCurDesc->fieldSize, 1) != 4))
					{
						fieldSize = pCurDesc->fieldSizeInBytes / 4;
						// fstream.WriteString(FormatA("digger %d %d ", pCurDesc->fieldSizeInBytes / 4, pCurDesc->fieldSizeInBytes / MAX(pCurDesc->fieldSize, 1)));
					}

					fstream.WriteString(FormatA("[%d]", fieldSize));
				}

				if (pCurDesc->td) // Print Type
				{
					if (pCurDesc->fieldSizeInBytes == 8 && DataMap_TotalSize(pCurDesc->td, 8) > 8)
						fstream.WriteString("*");
					fstream.WriteString(FormatA("server.%s", pCurDesc->td->dataClassName));
				}
				else
				{
					fstream.WriteString(FormatA("%s", DataMap_GetFieldTypeStr(pCurDesc)));
				}
			}
		}

		fstream.WriteString(FormatA(", // +0x%x size: 0x%x (0x%x * 0x%x) type %i\n", pCurDesc->fieldOffset, pCurDesc->fieldSizeInBytes, pCurDesc->fieldSize, pCurDesc->fieldSizeInBytes / MAX(pCurDesc->fieldSize, 1), pCurDesc->fieldType));
	}

	return lastPhysicalField;
}

void DataMap_DumpEmbeddedMaps(datamap_t*, std::unordered_set<std::string>&, CFileStream&);

//-----------------------------------------------------------------------------
// Purpose: Dumps a datamap_t pointer to a std::string passed by ref
// Input  : *pMap -
//          &svBuffer -
//-----------------------------------------------------------------------------
void DataMap_DumpStr(datamap_t* pMap, std::unordered_set<std::string>& structs, CFileStream& fstream)
{
	if (structs.count(pMap->dataClassName) == 0)
	{
		fstream.WriteString(FormatA("pub const %s = extern struct {\n", pMap->dataClassName));
		DataMap_DumpFields(pMap, fstream);
		fstream.WriteString("\n    test {\n");

		int alignment = DataMap_Alignment(pMap);
		int totalSize = DataMap_TotalSize(pMap, alignment);
		fstream.WriteString(FormatA("        try std.testing.expect(@sizeOf(@This()) == 0x%x);\n", totalSize));

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

			fstream.WriteString(FormatA("        try std.testing.expect(@offsetOf(@This(), \"%s\") == 0x%x);\n", TypeDesc_FieldName(pCurDesc->fieldName), pCurDesc->fieldOffset));
		}

		fstream.WriteString("    }\n};\n");

		structs.insert(pMap->dataClassName);
	}

	DataMap_DumpEmbeddedMaps(pMap, structs, fstream);
}

void DataMap_DumpMap(datamap_t* pMap, std::unordered_set<std::string>& structs)
{
	if (structs.count(pMap->dataClassName) > 0)
		return;

	CFileStream fStream;
	fStream.Open(FormatA("%s\\datamaps\\%s.zig", g_pEngineParms->szModName, pMap->dataClassName), CFileStream::WRITE);
	DataMap_DumpStr(pMap, structs, fStream);

	fStream.WriteString("\nconst std = @import(\"std\");\n"
						"const valve = @import(\"../../../vsource.zig\");\n"
						"const abi = @import(\"../../../abi.zig\");\n"
						"const m = @import(\"../../../math/vector.zig\");\n"
						"const server = @import(\"../server.zig\");");

	fStream.Close();
}

void DataMap_DumpEmbeddedMaps(datamap_t* pMap, std::unordered_set<std::string>& structs, CFileStream& fstream)
{
	int nFields = pMap->dataNumFields;
	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		if (pCurDesc->td && structs.count(pCurDesc->td->dataClassName) == 0)
		{
			// DataMap_DumpStr(pCurDesc->td, structs, fstream);
			DataMap_DumpMap(pCurDesc->td, structs);
		}
	}

	if (pMap->pBaseMap && structs.count(pMap->pBaseMap->dataClassName) == 0)
	{
		DataMap_DumpMap(pMap->pBaseMap, structs);
		// CFileStream baseMapStream;
		// baseMapStream.Open(FormatA("%s\\datamaps\\%s.zig", g_pEngineParms->szModName, pMap->pBaseMap->dataClassName), CFileStream::WRITE);

		// DataMap_DumpStr(pMap->pBaseMap, structs, baseMapStream);
		// DataMap_DumpEmbeddedMaps(pMap->pBaseMap, structs, baseMapStream);

		// baseMapStream.WriteString("\nconst std = @import(\"std\");\n"
		//						  "const valve = @import(\"../vsource.zig\");\n"
		//						  "const abi = @import(\"../abi.zig\");\n"
		//						  "const m = @import(\"../math/vector.zig\");\n"
		//						  "const server = @import(\"../server.zig\");");

		// baseMapStream.Close();
	}
}

void DataMap_WriteIndexImport(datamap_t* pMap, std::unordered_set<std::string>& structs, CFileStream& fstream)
{
	if (structs.count(pMap->dataClassName) > 0)
		return;
	structs.insert(pMap->dataClassName);

	fstream.WriteString(FormatA("pub const %s = @import(\"./server/%s.zig\").%s;\n", pMap->dataClassName, pMap->dataClassName, pMap->dataClassName));

	if (pMap->pBaseMap)
		DataMap_WriteIndexImport(pMap->pBaseMap, structs, fstream);

	int nFields = pMap->dataNumFields;
	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		if (pCurDesc->td && structs.count(pCurDesc->td->dataClassName) == 0)
		{
			DataMap_WriteIndexImport(pCurDesc->td, structs, fstream);
		}
	}
}

void DataMap_WriteIndex(std::vector<datamap_t*>& maps)
{
	CFileStream fstream;
	fstream.Open(FormatA("%s\\datamaps\\_server.zig", g_pEngineParms->szModName), CFileStream::WRITE);

	std::unordered_set<std::string> structs = {};

	for (datamap_t* pMap : maps)
	{
		DataMap_WriteIndexImport(pMap, structs, fstream);
	}

	fstream.WriteString("\ntest {\n"
						"    std.testing.refAllDecls(@This());\n"
						"}\n\n"
						"const std = @import(\"std\");\n");
	fstream.Close();
}

//-----------------------------------------------------------------------------
// Purpose: Dumps a datamap_t pointer to disk
// Input  : *pMap -
//-----------------------------------------------------------------------------
void DataMap_Dump(std::vector<datamap_t*> maps)
{
	DevMsg(eLog::NS, "Dumping %i datamaps\n", maps.size());

	std::unordered_set<std::string> structs = {};

	for (datamap_t* pMap : maps)
	{
		DataMap_DumpMap(pMap, structs);
	}

	DataMap_WriteIndex(maps);

	DevMsg(eLog::NS, "Successfully dumped datamaps\n");
}
