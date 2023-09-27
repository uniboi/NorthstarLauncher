#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

struct SQStructDef
{
	void* vtable;
	int uiRef;
	unsigned char padding_C[4];
	unsigned char unknown[24];
	SQSharedState* sharedState;
	SQObjectType _nameType;
	SQString* _name;
	unsigned char gap_38[16];
	SQObjectType _variableNamesType;
	SQTable* _variableNames;
	unsigned char gap_[32];
};

struct SQStructInstance
{
	void* vftable;
	__int32 uiRef;
	BYTE gap_C[4];
	__int64 unknown_10;
	void* pointer_18;
	__int64 unknown_20;
	SQSharedState* _sharedState;
	unsigned int size;
	BYTE gap_34[4];
	SQObject data[1]; // This struct is dynamically sized, so this size is unknown
};
