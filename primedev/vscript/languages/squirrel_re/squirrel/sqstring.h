#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

// NOTE [Fifty]: Variable sized struct
struct alignas(8) SQString
{
	void* vftable;
	int uiRef;
	int padding;
	SQString* _next_maybe;
	SQSharedState* sharedState;
	int length;
	unsigned char gap_24[4];
	char _hash[8];
	char _val[1];
};
static_assert(sizeof(SQString) == 56); // [Fifty]: Game allocates 56 + strlen
