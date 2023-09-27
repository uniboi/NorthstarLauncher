#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

struct SQArray
{
	void* vftable;
	int uiRef;
	unsigned char gap_24[36];
	SQObject* _values;
	int _usedSlots;
	int _allocated;
};
