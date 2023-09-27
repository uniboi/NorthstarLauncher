#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

// NOTE [Fifty]: Variable sized struct
struct SQUserData
{
	void* vftable;
	int uiRef;
	char gap_12[4];
	long long unknown_10;
	long long unknown_18;
	long long unknown_20;
	long long sharedState;
	long long unknown_30;
	int size;
	char padding1[4];
	void* (*releasehook)(void* val, int size);
	long long typeId;
	char data[1];
};
static_assert(sizeof(SQUserData) == 88);  // [Fifty]: Game allocates 87 + size (passed to the function)
