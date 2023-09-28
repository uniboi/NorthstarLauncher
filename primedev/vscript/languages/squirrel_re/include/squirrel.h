#pragma once

class Mod;
struct SQBufferState;
class CBaseEntity;

struct SQVM;
struct SQObject;
struct SQTable;
struct SQArray;
struct SQString;
struct SQClosure;
struct SQFunctionProto;
struct SQStructDef;
struct SQNativeClosure;
struct SQStructInstance;
struct SQUserData;
struct SQSharedState;

typedef float SQFloat;
typedef long SQInteger;
typedef unsigned long SQUnsignedInteger;
typedef char SQChar;
typedef SQUnsignedInteger SQBool;

typedef SQVM* HSQUIRRELVM;

enum SQRESULT : SQInteger
{
	SQRESULT_ERROR = -1,
	SQRESULT_NULL = 0,
	SQRESULT_NOTNULL = 1,
};

enum class eSQReturnType
{
	Float = 0x1,
	Vector = 0x3,
	Integer = 0x5,
	Boolean = 0x6,
	Entity = 0xD,
	String = 0x21,
	Default = 0x20,
	Arrays = 0x25,
	Asset = 0x28,
	Table = 0x26,
};

struct SQBufferState
{
	const SQChar* buffer;
	const SQChar* bufferPlusLength;
	const SQChar* bufferAgain;

	SQBufferState(const SQChar* pszCode)
	{
		buffer = pszCode;
		bufferPlusLength = pszCode + strlen(pszCode);
		bufferAgain = pszCode;
	}
};

typedef SQRESULT (*SQFunction)(HSQUIRRELVM sqvm);

struct SQFuncRegistration
{
	const char* squirrelFuncName;
	const char* cppFuncName;
	const char* helpText;
	const char* returnTypeString;
	const char* argTypes;
	uint32_t unknown1;
	uint32_t devLevel;
	const char* shortNameMaybe;
	uint32_t unknown2;
	eSQReturnType returnType;
	uint32_t* externalBufferPointer;
	uint64_t externalBufferSize;
	uint64_t unknown3;
	uint64_t unknown4;
	SQFunction funcPtr;

	SQFuncRegistration()
	{
		memset(this, 0, sizeof(SQFuncRegistration));
		this->returnType = eSQReturnType::Default;
	}
};

struct alignas(8) SQStackInfos
{
	char* _name;
	char* _sourceName;
	int _line;
};

//-----------------------------------------------------------------------------
// CLIENT.DLL
//-----------------------------------------------------------------------------
inline const SQChar* (*o_sq_getstringClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQInteger (*o_sq_getintegerClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQFloat (*o_sq_getfloatClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQBool (*o_sq_getboolClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQRESULT (*o_sq_getClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQRESULT (*o_sq_getassetClient)(HSQUIRRELVM sqvm, SQInteger iStackpos, const char** pResult);
inline SQRESULT (*o_sq_getuserdataClient)(HSQUIRRELVM sqvm, SQInteger iStackpos, void** pData, uint64_t* pId);
inline SQFloat* (*o_sq_getvectorClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQBool (*o_sq_getthisentityClient)(HSQUIRRELVM sqvm, void** ppEntity);
inline void (*o_sq_getobjectClient)(HSQUIRRELVM sqvm, SQInteger iStackPos, SQObject* pOutObj);
inline int (*o_sq_getfunctionClient)(HSQUIRRELVM sqvm, const char* name, SQObject* returnObj, const char* signature);

inline void (*o_sq_pushroottableClient)(HSQUIRRELVM sqvm);
inline void (*o_sq_pushstringClient)(HSQUIRRELVM sqvm, const SQChar* pStr, SQInteger iLength);
inline void (*o_sq_pushintegerClient)(HSQUIRRELVM sqvm, SQInteger i);
inline void (*o_sq_pushfloatClient)(HSQUIRRELVM sqvm, SQFloat f);
inline void (*o_sq_pushboolClient)(HSQUIRRELVM sqvm, SQBool b);
inline void (*o_sq_pushassetClient)(HSQUIRRELVM sqvm, const SQChar* str, SQInteger iLength);
inline void (*o_sq_pushvectorClient)(HSQUIRRELVM sqvm, const SQFloat* pVec);
inline void (*o_sq_pushobjectClient)(HSQUIRRELVM sqvm, SQObject* pObj);

inline void (*o_sq_newarrayClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQRESULT (*o_sq_arrayappendClient)(HSQUIRRELVM sqvm, SQInteger iStackpos);

inline SQRESULT (*o_sq_newtableClient)(HSQUIRRELVM sqvm);
inline SQRESULT (*o_sq_newslotClient)(HSQUIRRELVM sqvm, SQInteger idx, SQBool bStatic);

inline SQRESULT (*o_sq_pushnewstructinstanceClient)(HSQUIRRELVM sqvm, int fieldCount);
inline SQRESULT (*o_sq_sealstructslotClient)(HSQUIRRELVM sqvm, int slotIndex);

inline void* (*o_sq_createuserdataClient)(HSQUIRRELVM sqvm, SQInteger iSize);
inline SQRESULT (*o_sq_setuserdatatypeidClient)(HSQUIRRELVM sqvm, SQInteger iStackpos, uint64_t iTypeId);

inline long long (*o_sq_stackinfosClient)(HSQUIRRELVM sqvm, int iLevel, SQStackInfos* pOutObj, int iCallStackSize);

inline SQRESULT (*o_sq_compilebufferClient)(HSQUIRRELVM sqvm, SQBufferState* compileBuffer, const char* file, int a1, SQBool bShouldThrowError);
inline SQRESULT (*o_sq_callClient)(HSQUIRRELVM sqvm, SQInteger iArgs, SQBool bShouldReturn, SQBool bThrowError);
inline SQInteger (*o_sq_raiseerrorClient)(HSQUIRRELVM sqvm, const SQChar* pError);

//-----------------------------------------------------------------------------
// SERVER.DLL
//-----------------------------------------------------------------------------
inline const SQChar* (*o_sq_getstringServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQInteger (*o_sq_getintegerServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQFloat (*o_sq_getfloatServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQBool (*o_sq_getboolServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQRESULT (*o_sq_getServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQRESULT (*o_sq_getassetServer)(HSQUIRRELVM sqvm, SQInteger iStackpos, const char** pResult);
inline SQRESULT (*o_sq_getuserdataServer)(HSQUIRRELVM sqvm, SQInteger iStackpos, void** pData, uint64_t* pId);
inline SQFloat* (*o_sq_getvectorServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQBool (*o_sq_getthisentityServer)(HSQUIRRELVM sqvm, void** ppEntity);
inline void (*o_sq_getobjectServer)(HSQUIRRELVM sqvm, SQInteger iStackPos, SQObject* pOutObj);
inline int (*o_sq_getfunctionServer)(HSQUIRRELVM sqvm, const char* name, SQObject* returnObj, const char* signature);

inline void (*o_sq_pushroottableServer)(HSQUIRRELVM sqvm);
inline void (*o_sq_pushstringServer)(HSQUIRRELVM sqvm, const SQChar* pStr, SQInteger iLength);
inline void (*o_sq_pushintegerServer)(HSQUIRRELVM sqvm, SQInteger i);
inline void (*o_sq_pushfloatServer)(HSQUIRRELVM sqvm, SQFloat f);
inline void (*o_sq_pushboolServer)(HSQUIRRELVM sqvm, SQBool b);
inline void (*o_sq_pushassetServer)(HSQUIRRELVM sqvm, const SQChar* str, SQInteger iLength);
inline void (*o_sq_pushvectorServer)(HSQUIRRELVM sqvm, const SQFloat* pVec);
inline void (*o_sq_pushobjectServer)(HSQUIRRELVM sqvm, SQObject* pObj);

inline void (*o_sq_newarrayServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);
inline SQRESULT (*o_sq_arrayappendServer)(HSQUIRRELVM sqvm, SQInteger iStackpos);

inline SQRESULT (*o_sq_newtableServer)(HSQUIRRELVM sqvm);
inline SQRESULT (*o_sq_newslotServer)(HSQUIRRELVM sqvm, SQInteger idx, SQBool bStatic);

inline SQRESULT (*o_sq_pushnewstructinstanceServer)(HSQUIRRELVM sqvm, int fieldCount);
inline SQRESULT (*o_sq_sealstructslotServer)(HSQUIRRELVM sqvm, int slotIndex);

inline void* (*o_sq_createuserdataServer)(HSQUIRRELVM sqvm, SQInteger iSize);
inline SQRESULT (*o_sq_setuserdatatypeidServer)(HSQUIRRELVM sqvm, SQInteger iStackpos, uint64_t iTypeId);

inline long long (*o_sq_stackinfosServer)(HSQUIRRELVM sqvm, int iLevel, SQStackInfos* pOutObj, int iCallStackSize);

inline SQRESULT (*o_sq_compilebufferServer)(HSQUIRRELVM sqvm, SQBufferState* compileBuffer, const char* file, int a1, SQBool bShouldThrowError);
inline SQRESULT (*o_sq_callServer)(HSQUIRRELVM sqvm, SQInteger iArgs, SQBool bShouldReturn, SQBool bThrowError);
inline SQInteger (*o_sq_raiseerrorServer)(HSQUIRRELVM sqvm, const SQChar* pError);

//-----------------------------------------------------------------------------
// WRAPPERS
//-----------------------------------------------------------------------------
const SQChar* sq_getstring(HSQUIRRELVM sqvm, SQInteger iStackpos);
SQInteger sq_getinteger(HSQUIRRELVM sqvm, SQInteger iStackpos);
SQFloat sq_getfloat(HSQUIRRELVM, SQInteger iStackpos);
SQBool sq_getbool(HSQUIRRELVM, SQInteger iStackpos);
SQRESULT sq_get(HSQUIRRELVM sqvm, SQInteger iStackpos);
SQRESULT sq_getasset(HSQUIRRELVM sqvm, SQInteger iStackpos, const char** pResult);
SQRESULT sq_getuserdata(HSQUIRRELVM sqvm, SQInteger iStackpos, void** pData, uint64_t* pId);
SQFloat* sq_getvector(HSQUIRRELVM sqvm, SQInteger iStackpos);
SQBool sq_getthisentity(HSQUIRRELVM sqvm, void** ppEntity);
void sq_getobject(HSQUIRRELVM sqvm, SQInteger iStackPos, SQObject* pOutObj);
int sq_getfunction(HSQUIRRELVM sqvm, const char* pszName, SQObject* pReturnObj, const char* pszSignature);

void sq_pushroottable(HSQUIRRELVM sqvm);
void sq_pushstring(HSQUIRRELVM sqvm, const SQChar* pStr, SQInteger iLength);
void sq_pushinteger(HSQUIRRELVM sqvm, SQInteger i);
void sq_pushfloat(HSQUIRRELVM sqvm, SQFloat f);
void sq_pushbool(HSQUIRRELVM sqvm, SQBool b);
void sq_pushasset(HSQUIRRELVM sqvm, const SQChar* str, SQInteger iLength);
void sq_pushvector(HSQUIRRELVM sqvm, const SQFloat* pVec);
void sq_pushobject(HSQUIRRELVM sqvm, SQObject* pObj);

void sq_newarray(HSQUIRRELVM sqvm, SQInteger iStackpos);
SQRESULT sq_arrayappend(HSQUIRRELVM sqvm, SQInteger iStackpos);

SQRESULT sq_newtable(HSQUIRRELVM sqvm);
SQRESULT sq_newslot(HSQUIRRELVM sqvm, SQInteger idx, SQBool bStatic);

SQRESULT sq_pushnewstructinstance(HSQUIRRELVM sqvm, int fieldCount);
SQRESULT sq_sealstructslot(HSQUIRRELVM sqvm, int slotIndex);

void* sq_createuserdata(HSQUIRRELVM sqvm, SQInteger iSize);
SQRESULT sq_setuserdatatypeid(HSQUIRRELVM sqvm, SQInteger iStackpos, uint64_t iTypeId);

long long sq_stackinfos(HSQUIRRELVM sqvm, int iLevel, SQStackInfos* pOutObj);

SQRESULT sq_compilebuffer(HSQUIRRELVM sqvm, SQBufferState* compileBuffer, const char* file, int a1, SQBool bShouldThrowError);
SQRESULT sq_call(HSQUIRRELVM sqvm, SQInteger iArgs, SQBool bShouldReturn, SQBool bThrowError);
SQInteger sq_raiseerror(HSQUIRRELVM sqvm, const SQChar* pError);

//-----------------------------------------------------------------------------
// CUSTOM FUNCTIONS
//-----------------------------------------------------------------------------
Mod* sq_getcallingmod(HSQUIRRELVM sqvm, int depth = 0);
CBaseEntity* sq_getentity(HSQUIRRELVM sqvm, SQInteger iStackPos);
