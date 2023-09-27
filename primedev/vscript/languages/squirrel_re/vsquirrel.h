#pragma once

#include "vscript/languages/squirrel_re/include/squirrel.h"
#include "vscript/languages/squirrel_re/squirrel/sqvm.h"

eLog VScript_GetNativeLogContext(ScriptContext nContext);
const char* VScript_GetContextAsString(ScriptContext nContext);
eSQReturnType SQ_GetTypeFromString(const char* pReturnType);
const char* SQ_GetTypeAsString(const int iTypeId);

class CSquirrelVM;

inline bool (*o_CSquirrelVM__InitClient)(CSquirrelVM* vm, ScriptContext nContext, float time);
inline bool (*o_CSquirrelVM__InitServer)(CSquirrelVM* vm, ScriptContext nContext, float time);

inline void (*o_CSquirrelVM__DefConstClient)(CSquirrelVM* sqvm, const SQChar* name, int value);
inline void (*o_CSquirrelVM__DefConstServer)(CSquirrelVM* sqvm, const SQChar* name, int value);

inline int64_t (*o_CSquirrelVM__RegisterFunctionClient)(CSquirrelVM* sqvm, SQFuncRegistration* pFuncReg, char unknown);
inline int64_t (*o_CSquirrelVM__RegisterFunctionServer)(CSquirrelVM* sqvm, SQFuncRegistration* pFuncReg, char unknown);

inline bool (*o_CSquirrelVM__CompileFileClient)(CSquirrelVM* sqvm, const char* path, const char* name, int a4);
inline bool (*o_CSquirrelVM__CompileFileServer)(CSquirrelVM* sqvm, const char* path, const char* name, int a4);

inline void* (*o_CSquirrelVM__GetEntityInstanceClient)(CSquirrelVM* sqvm, SQObject* pInstance, char** ppEntityConstant);
inline void* (*o_CSquirrelVM__GetEntityInstanceServer)(CSquirrelVM* sqvm, SQObject* pInstance, char** ppEntityConstant);

inline char** (*o_CSquirrelVM__GetEntityConstantClient)();
inline char** (*o_CSquirrelVM__GetEntityConstantServer)();

inline bool (*o_CSquirrelVM__CallScriptInitCallbackClient)(CSquirrelVM* sqvm, const char* pszCallback);
inline bool (*o_CSquirrelVM__CallScriptInitCallbackServer)(CSquirrelVM* sqvm, const char* pszCallback);

class CSquirrelVM
{
  public:
	inline HSQUIRRELVM GetVM()
	{
		return m_hVM;
	}

	inline void AddConstant(const SQChar* pszName, int nValue)
	{
		if (vmContext == (int)ScriptContext::SERVER)
			o_CSquirrelVM__DefConstServer(this, pszName, nValue);
		else
			o_CSquirrelVM__DefConstClient(this, pszName, nValue);
	}

	inline void RegisterFunction(SQFuncRegistration* pFuncReg, char unknown)
	{
		if (vmContext == (int)ScriptContext::SERVER)
			o_CSquirrelVM__RegisterFunctionServer(this, pFuncReg, unknown);
		else
			o_CSquirrelVM__RegisterFunctionClient(this, pFuncReg, unknown);
	}

	inline void RegisterFunction(const char* pszScriptName, const char* pszNativeName, const char* pszHelpText, const char* pszReturnType, const char* pszParameters, SQFunction fnFunction)
	{
		SQFuncRegistration reg;

		reg.squirrelFuncName = pszScriptName;
		reg.cppFuncName = pszNativeName;
		reg.helpText = pszHelpText;
		reg.returnTypeString = pszReturnType;
		reg.returnType = SQ_GetTypeFromString(pszReturnType);
		reg.argTypes = pszParameters;
		reg.funcPtr = fnFunction;

		RegisterFunction(&reg, 1);
	}

	inline bool CompileFile(const SQChar* pszPath, const SQChar* pszName, int nUnk)
	{
		if (vmContext == (int)ScriptContext::SERVER)
			return o_CSquirrelVM__CompileFileServer(this, pszPath, pszName, nUnk);
		else
			return o_CSquirrelVM__CompileFileClient(this, pszPath, pszName, nUnk);
	}

	inline void* GetEntityFromObject(SQObject* pObj)
	{
		if (vmContext == (int)ScriptContext::SERVER)
			return o_CSquirrelVM__GetEntityInstanceServer(this, pObj, o_CSquirrelVM__GetEntityConstantServer());
		else
			return o_CSquirrelVM__GetEntityInstanceClient(this, pObj, o_CSquirrelVM__GetEntityConstantClient());
	}

	inline void ExecuteBuffer(const SQChar* pszBuf)
	{
		ScriptContext nContext = (ScriptContext)vmContext;
		if (!GetVM())
		{
			Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Cannot execute code, %s squirrel vm is not initialised\n", VScript_GetContextAsString(nContext));
			return;
		}

		DevMsg(VScript_GetNativeLogContext(nContext), "Executing %s script code %s\n", VScript_GetContextAsString(nContext), pszBuf);

		SQBufferState bufferState = SQBufferState(pszBuf);

		SQRESULT compileResult = sq_compilebuffer(GetVM(), &bufferState, "console", -1, false);

		if (compileResult == SQRESULT_ERROR)
		{
			Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Error compiling buffer: '%s'\n", GetVM()->GetLastError());
		}
		else
		{
			sq_pushroottable(GetVM());

			SQRESULT callResult = sq_call(GetVM(), 1, false, false);
			if (callResult == SQRESULT_ERROR)
			{
				Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Error calling buffer: '%s'\n", GetVM()->GetLastError());
			}
		}
	}

	void CallDestroyCallbacks();

  public:
	char gap[8];
	HSQUIRRELVM m_hVM;
	BYTE gap_10[8];
	SQObject unknownObject_18;
	__int64 unknown_28;
	BYTE gap_30[12];
	__int32 vmContext;
	BYTE gap_40[648];
	char* (*formatString)(__int64 a1, const char* format, ...);
	BYTE gap_2D0[24];
};

inline CSquirrelVM* g_pServerVM = nullptr;
inline CSquirrelVM* g_pClientVM = nullptr;
inline CSquirrelVM* g_pUIVM = nullptr;
