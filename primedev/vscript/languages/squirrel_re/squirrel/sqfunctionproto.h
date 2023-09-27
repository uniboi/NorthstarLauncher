#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

struct alignas(8) SQFunctionProto
{
	void* vftable;
	unsigned char gap_08[4];
	int uiRef;
	unsigned char gap_10[8];
	void* pointer_18;
	void* pointer_20;
	void* sharedState;
	void* pointer_30;
	SQObjectType _fileNameType;
	SQString* _fileName;
	SQObjectType _funcNameType;
	SQString* _funcName;
	SQObject obj_58;
	unsigned char gap_68[12];
	int _stacksize;
	unsigned char gap_78[48];
	int nParameters;
	unsigned char gap_AC[60];
	int nDefaultParams;
	unsigned char gap_EC[200];
};
