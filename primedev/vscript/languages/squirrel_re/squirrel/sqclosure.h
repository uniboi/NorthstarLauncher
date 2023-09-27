#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

struct alignas(8) SQClosure
{
	void* vftable;
	unsigned char gap_08[4];
	int uiRef;
	void* pointer_10;
	void* pointer_18;
	void* pointer_20;
	void* sharedState;
	SQObject obj_30;
	SQObject _function;
	SQObject* _outervalues;
	unsigned char gap_58[8];
	unsigned char gap_60[96];
	SQObject* objectPointer_C0;
	unsigned char gap_C8[16];
};

struct alignas(8) SQNativeClosure
{
	void* vftable;
	int uiRef;
	unsigned char gap_C[4];
	long long value_10;
	long long value_18;
	long long value_20;
	SQSharedState* sharedState;
	char unknown_30;
	unsigned char padding_34[7];
	long long value_38;
	long long value_40;
	long long value_48;
	long long value_50;
	long long value_58;
	SQObjectType _nameType;
	SQString* _name;
	long long value_70;
	long long value_78;
	unsigned char justInCaseGap_80[300];
};
