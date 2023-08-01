#pragma once

typedef long long SQInteger;
typedef int SQInt32; /*must be 32 bits(also on 64bits processors)*/
typedef unsigned long long SQUnsignedInteger;
typedef unsigned long long SQHash; /*should be the same size of a pointer*/
typedef SQUnsignedInteger SQBool;

enum SQRESULT
{
	SQRESULT_NULL = 0,
	SQRESULT_NOTNULL = 1,
	SQRESULT_ERROR = -1,
};
