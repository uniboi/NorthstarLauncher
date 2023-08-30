#include "engine/datamap.h"

#include "tier0/filestream.h"
#include "engine/host.h"

#include <sstream>

//-----------------------------------------------------------------------------
// Purpose: Returns a string from fieldtype_t enum entry
// Input  : type -
//-----------------------------------------------------------------------------
const char* DataMap_GetFieldTypeStr(fieldtype_t type)
{
	switch (type)
	{
	case FIELD_VOID:
		return "void";
	case FIELD_FLOAT:
		return "float";
	case FIELD_STRING:
		return "__int64";
	case FIELD_VECTOR:
		return "Vector3";
	case FIELD_QUATERNION:
		return "Quaternion";
	case FIELD_INTEGER:
		return "int";
	case FIELD_BOOLEAN:
		return "bool";
	case FIELD_SHORT:
		return "short";
	case FIELD_CHARACTER:
		return "char";
	case FIELD_COLOR32:
		return "Color32";
	case FIELD_EMBEDDED:
		return "embedded";
	case FIELD_CUSTOM:
		return "custom";
	case FIELD_CLASSPTR:
		return "CBaseEntity*";
	case FIELD_EHANDLE:
		return "EHANDLE";
	case FIELD_EDICT:
		return "edict_t*";
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
		return "matrix3x4_t";
	case FIELD_INTERVAL:
		return "Interval";
	case FIELD_MODELINDEX:
		return "ModelIndex";
	case FIELD_MATERIALINDEX:
		return "MatIndex";
	case FIELD_VECTOR2D:
		return "Vector2";
	case FIELD_INTEGER64:
		return "__int64";
	case FIELD_VECTOR4D:
		return "Vector4";
	}

	return "unk";
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
		return "char";
	case 2:
		return "short";
	case 4:
		return "int";
	case 8:
		return "__int64";
	}

	return "unk";
}

//-----------------------------------------------------------------------------
// Purpose: Dumps a datamap_t pointer to a std::string passed by ref
// Input  : *pMap -
//          &svBuffer -
//-----------------------------------------------------------------------------
void DataMap_DumpStr(datamap_t* pMap, std::string& svBuffer)
{
	svBuffer += FormatA("class %s", pMap->dataClassName);

	if (pMap->pBaseMap)
	{
		svBuffer += FormatA(" : public %s", pMap->pBaseMap->dataClassName);
	}

	svBuffer += "\n{\n";

	int nFields = pMap->dataNumFields;
	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		// var
		if (pCurDesc->td) // Print Type
		{
			svBuffer += FormatA("\t%s ", pCurDesc->td->dataClassName);
		}
		else
		{
			svBuffer += FormatA("\t%s ", DataMap_GetFieldTypeStr(pCurDesc->fieldType));
		}

		svBuffer += pCurDesc->fieldName; // Print name
		if (pCurDesc->fieldSize != 1)
		{
			svBuffer += FormatA(" [%d]", pCurDesc->fieldSize); // Print array size
		}
		svBuffer += FormatA("; // 0x%x ( Size: %d )\n", pCurDesc->fieldOffset, pCurDesc->fieldSizeInBytes); // Print offset

		// pad
		if (i - 1 == nFields)
			continue;

		typedescription_t* pNextDesc = &pMap->dataDesc[i + 1];

		if (pCurDesc->fieldOffset + pCurDesc->fieldSizeInBytes != pNextDesc->fieldOffset)
		{
			svBuffer += FormatA("\tchar gap_%x[%i];\n", pCurDesc->fieldOffset + pCurDesc->fieldSizeInBytes, pNextDesc->fieldOffset - pCurDesc->fieldOffset - pCurDesc->fieldSizeInBytes);
		}
	}

	svBuffer += "};\n\n";

	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		if (pCurDesc->td)
		{
			DataMap_DumpStr(pCurDesc->td, svBuffer);
		}
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

	// dump our map
	DataMap_DumpStr(pMap, svMapFile);

	// Write it to disk
	CFileStream fStream;
	fStream.Open(FormatA("%s\\Map_%s.txt", g_pEngineParms->szModName, pMap->dataClassName).c_str(), CFileStream::WRITE);
	fStream.WriteString(svAddTypes);
	fStream.WriteString(svMapFile);
	fStream.Close();

	DevMsg(eLog::NS, "Succesfully dumped %s\n", pMap->dataClassName);

	if (pMap->pBaseMap)
	{
		DataMap_Dump(pMap->pBaseMap);
	}
}
