#include "engine/datamap.h"

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
		return "Vector";
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
		return "color32";
	case FIELD_EMBEDDED:
		return "embedded";
	case FIELD_CUSTOM:
		return "custom";
	case FIELD_CLASSPTR:
		return "CBaseEntity*";
	case FIELD_EHANDLE:
		return "ehandle";
	case FIELD_EDICT:
		return "edict_t*";
	case FIELD_POSITION_VECTOR:
		return "PosVec";
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
		return "Matrix3x4Wrld";
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
// Purpose: Dumps a datamap_t pointer by logging its contents
// Input  : *pMap -
//-----------------------------------------------------------------------------
void DataMap_Dump(datamap_t* pMap)
{
	// TODO [Fifty]: Dump to file instead of directly logging the thing

	if (!pMap)
	{
		Error(eLog::ENGINE, NO_ERROR, "Datamap is null!");
		return;
	}

	DevMsg(eLog::NONE, "Dumping datamap\n");

	std::stringstream ss;

	// Create class structure as a stringstream
	ss << "class " << pMap->dataClassName << "\n{\n";

	int nFields = pMap->dataNumFields;
	for (int i = 0; i < nFields; i++)
	{
		typedescription_t* pCurDesc = &pMap->dataDesc[i];

		// var
		ss << "\t" << DataMap_GetSizeTypeStr(pCurDesc->fieldSizeInBytes / pCurDesc->fieldSize) << " "; // Print type
		ss << pCurDesc->fieldName; // Print name
		if (pCurDesc->fieldSize != 1)
		{
			ss << " [" << pCurDesc->fieldSize << "]"; // Print array size
		}
		ss << "; // " << pCurDesc->fieldOffset << "\n"; // Print offset

		// pad
		if (i - 1 == nFields)
			continue;

		typedescription_t* pNextDesc = &pMap->dataDesc[i + 1];

		if (pCurDesc->fieldOffset + pCurDesc->fieldSizeInBytes != pNextDesc->fieldOffset)
		{
			ss << "\tchar gap_" << pCurDesc->fieldOffset + pCurDesc->fieldSizeInBytes << "[" << pNextDesc->fieldOffset - pCurDesc->fieldOffset - pCurDesc->fieldSizeInBytes << "];\n";
		}
	}

	ss << "};\n";

	// Log it
	DevMsg(eLog::NONE, "\n%s\n", ss.str().c_str());
}
