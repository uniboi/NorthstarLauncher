#pragma once

// Forward declarations
struct datamap_t;
struct typedescription_t;
enum fieldtype_t;

//-----------------------------------------------------------------------------
// Helpers
const char* DataMap_GetFieldTypeStr(fieldtype_t type);
const char* DataMap_GetSizeTypeStr(int iBytes);

//-----------------------------------------------------------------------------
//
void DataMap_Dump(datamap_t* pMap);

//-----------------------------------------------------------------------------
// field types
enum fieldtype_t
{
	FIELD_VOID = 0, // No type or value
	FIELD_FLOAT, // Any floating point value
	FIELD_STRING, // A string ID (return from ALLOC_STRING)
	FIELD_VECTOR, // Any vector, QAngle, or AngularImpulse
	FIELD_QUATERNION, // A quaternion
	FIELD_INTEGER, // Any integer or enum
	FIELD_BOOLEAN, // boolean, implemented as an int, I may use this as a hint for compression
	FIELD_SHORT, // 2 byte integer
	FIELD_CHARACTER, // a byte
	FIELD_COLOR32, // 8-bit per channel r,g,b,a (32bit color)
	FIELD_EMBEDDED, // an embedded object with a datadesc, recursively traverse and embedded class/structure based on an additional typedescription
	FIELD_CUSTOM, // special type that contains function pointers to it's read/write/parse functions

	FIELD_CLASSPTR, // CBaseEntity *
	FIELD_EHANDLE, // Entity handle
	FIELD_EDICT, // edict_t *

	FIELD_POSITION_VECTOR, // A world coordinate (these are fixed up across level transitions automagically)
	FIELD_TIME, // a floating point time (these are fixed up automatically too!)
	FIELD_TICK, // an integer tick count( fixed up similarly to time)
	FIELD_MODELNAME, // Engine string that is a model name (needs precache)
	FIELD_SOUNDNAME, // Engine string that is a sound name (needs precache)

	FIELD_INPUT, // a list of inputed data fields (all derived from CMultiInputVar)
	FIELD_FUNCTION, // A class function pointer (Think, Use, etc)

	FIELD_VMATRIX, // a vmatrix (output coords are NOT worldspace)

	// NOTE: Use float arrays for local transformations that don't need to be fixed up.
	FIELD_VMATRIX_WORLDSPACE, // A VMatrix that maps some local space to world space (translation is fixed up on level transitions)
	FIELD_MATRIX3X4_WORLDSPACE, // matrix3x4_t that maps some local space to world space (translation is fixed up on level transitions)

	FIELD_INTERVAL, // a start and range floating point interval ( e.g., 3.2->3.6 == 3.2 and 0.4 )
	FIELD_MODELINDEX, // a model index
	FIELD_MATERIALINDEX, // a material index (using the material precache string table)

	FIELD_VECTOR2D, // 2 floats
	FIELD_INTEGER64, // 64bit integer

	FIELD_VECTOR4D, // 4 floats

	FIELD_TYPECOUNT, // MUST BE LAST
};

//-----------------------------------------------------------------------------
// field description
struct typedescription_t
{
	fieldtype_t fieldType; // 4
	const char* fieldName; // 8
	int fieldOffset; // Local offset value // 4
	unsigned short fieldSize; // 2
	short flags; // 2
	// the name of the variable in the map/fgd data, or the name of the action
	const char* externalName; // 8
	// pointer to the function set for save/restoring of custom data types
	void /*ISaveRestoreOps*/* pSaveRestoreOps; // 8
	// for associating function with string names
	void /*inputfunc_t*/* inputFunc; // 8
	// For embedding additional datatables inside this one
	datamap_t* td; // 8

	// Stores the actual member variable size in bytes
	int fieldSizeInBytes; // 4

	// FTYPEDESC_OVERRIDE point to first baseclass instance if chains_validated has occurred
	struct typedescription_t* override_field; // 8

	// Used to track exclusion of baseclass fields
	int override_count; // 4

	// Tolerance for field errors for float fields
	float fieldTolerance; // 4

	// For raw fields (including children of embedded stuff) this is the flattened offset
	int flatOffset[2]; // 8
	unsigned short flatGroup; // 2
	char pad[14];
};
static_assert(sizeof(typedescription_t) == 104);

//-----------------------------------------------------------------------------
// Purpose: stores the list of objects in the hierarchy
//			used to iterate through an object's data descriptions
//-----------------------------------------------------------------------------
struct datamap_t
{
	typedescription_t* dataDesc;
	int dataNumFields;
	char const* dataClassName;
	datamap_t* baseMap;

	int m_nPackedSize;
	void* m_pOptimizedDataMap;
};
