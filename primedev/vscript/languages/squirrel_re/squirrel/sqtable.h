#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"

struct alignas(8) SQTable
{
	struct _HashNode
	{
		SQObject val;
		SQObject key;
		_HashNode* next;
	};

	void* vftable;
	unsigned char gap_08[4];
	int uiRef;
	unsigned char gap_10[8];
	void* pointer_18;
	void* pointer_20;
	void* _sharedState;
	long long field_30;
	_HashNode* _nodes;
	int _numOfNodes;
	int size;
	int field_48;
	int _usedNodes;
	unsigned char _gap_50[20];
	int field_64;
	unsigned char _gap_68[80];
};
