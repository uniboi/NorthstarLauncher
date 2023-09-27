#include "vscript/languages/squirrel_re/vsquirrel.h"

#include "vscript/languages/squirrel_re/squirrel/sqstate.h"
#include "mods/modmanager.h"

void CSquirrelVM::CallDestroyCallbacks()
{
	if (g_pModManager)
	{
		DevMsg(VScript_GetNativeLogContext((ScriptContext)vmContext), "Calling Destroy callbacks for all loaded mods.\n");

		for (const Mod& loadedMod : g_pModManager->m_LoadedMods)
		{
			for (const ModScript& script : loadedMod.Scripts)
			{
				for (const ModScriptCallback& callback : script.Callbacks)
				{
					if (callback.Context != (ScriptContext)vmContext || callback.DestroyCallback.length() == 0)
					{
						continue;
					}

					if (GetVM())
					{
						ScriptContext nContext = (ScriptContext)vmContext;
						const char* pszFuncName = callback.DestroyCallback.c_str();

						SQObject oFunction {};
						int nResult = sq_getfunction(GetVM(), pszFuncName, &oFunction, 0);
						if (nResult != 0)
						{
							Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFuncName);
							continue;
						}

						// Push
						sq_pushobject(GetVM(), &oFunction);
						sq_pushroottable(GetVM());

						(void)sq_call(GetVM(), 1, false, false);
					}

					DevMsg(VScript_GetNativeLogContext((ScriptContext)vmContext), "Executed Destroy callback %s.\n", callback.DestroyCallback.c_str());
				}
			}
		}
	}
}

eSQReturnType SQ_GetTypeFromString(const char* pReturnType)
{
	if (!strcmp(pReturnType, "bool"))
		return eSQReturnType::Boolean;
	if (!strcmp(pReturnType, "float"))
		return eSQReturnType::Float;
	if (!strcmp(pReturnType, "vector"))
		return eSQReturnType::Vector;
	if (!strcmp(pReturnType, "int"))
		return eSQReturnType::Integer;
	if (!strcmp(pReturnType, "entity"))
		return eSQReturnType::Entity;
	if (!strcmp(pReturnType, "string"))
		return eSQReturnType::String;
	if (!strcmp(pReturnType, "array"))
		return eSQReturnType::Arrays;
	if (!strcmp(pReturnType, "asset"))
		return eSQReturnType::Asset;
	if (!strcmp(pReturnType, "table"))
		return eSQReturnType::Table;

	return eSQReturnType::Default; // previous default value
}

const char* SQ_GetTypeAsString(int type)
{
	switch (type)
	{
	case OT_ASSET:
		return "asset";
	case OT_INTEGER:
		return "int";
	case OT_BOOL:
		return "bool";
	case SQOBJECT_NUMERIC:
		return "float or int";
	case OT_NULL:
		return "null";
	case OT_VECTOR:
		return "vector";
	case 0:
		return "var";
	case OT_USERDATA:
		return "userdata";
	case OT_FLOAT:
		return "float";
	case OT_STRING:
		return "string";
	case OT_ARRAY:
		return "array";
	case 0x8000200:
		return "function";
	case 0x8100000:
		return "structdef";
	case OT_THREAD:
		return "thread";
	case OT_FUNCPROTO:
		return "function";
	case OT_CLAAS:
		return "class";
	case OT_WEAKREF:
		return "weakref";
	case 0x8080000:
		return "unimplemented function";
	case 0x8200000:
		return "struct instance";
	case OT_TABLE:
		return "table";
	case 0xA008000:
		return "instance";
	case OT_ENTITY:
		return "entity";
	}
	return "";
}

const char* VScript_GetContextAsString(ScriptContext nContext);

bool h_CSquirrelVM__InitClient(CSquirrelVM* vm, ScriptContext nContext, float time)
{
	bool ret = o_CSquirrelVM__InitClient(vm, nContext, time);

	for (Mod mod : g_pModManager->m_LoadedMods)
	{
		if (mod.m_bEnabled && mod.initScript.size() != 0)
		{
			std::string name = mod.initScript.substr(mod.initScript.find_last_of('/') + 1);
			std::string path = std::string("scripts/vscripts/") + mod.initScript;

			if (vm->CompileFile(path.c_str(), name.c_str(), 0))
				vm->CompileFile(path.c_str(), name.c_str(), 1);
		}
	}

	return ret;
}

bool h_CSquirrelVM__InitServer(CSquirrelVM* vm, ScriptContext realContext, float time)
{
	bool ret = o_CSquirrelVM__InitServer(vm, realContext, time);

	for (Mod mod : g_pModManager->m_LoadedMods)
	{
		if (mod.m_bEnabled && mod.initScript.size() != 0)
		{
			std::string name = mod.initScript.substr(mod.initScript.find_last_of('/') + 1);
			std::string path = std::string("scripts/vscripts/") + mod.initScript;

			if (vm->CompileFile(path.c_str(), name.c_str(), 0))
				vm->CompileFile(path.c_str(), name.c_str(), 1);
		}
	}

	return ret;
}

SQRESULT Script_StubbedFunc(HSQUIRRELVM sqvm)
{
	SQStackInfos si;
	sq_stackinfos(sqvm, 0, &si);

	ScriptContext nContext = (ScriptContext)sqvm->sharedState->cSquirrelVM->vmContext;

	Warning(VScript_GetNativeLogContext(nContext), "Blocking call to stubbed function %s in %s\n", si._name, VScript_GetContextAsString(nContext));
	return SQRESULT_NULL;
}

int64_t h_CSquirrelVM__RegisterFunction(CSquirrelVM* sqvm, SQFuncRegistration* pFuncReg, char unknown)
{
	ScriptContext nContext = (ScriptContext)sqvm->vmContext;

	const char* ppUnsafeFuncs[] = {"DevTextBufferWrite", "DevTextBufferClear", "DevTextBufferDumpToFile", "Dev_CommandLineAddParam", "DevP4Checkout", "DevP4Add"};

	for (int i = 0; i < ARRAY_SIZE(ppUnsafeFuncs); i++)
	{
		if (!strcmp(pFuncReg->squirrelFuncName, ppUnsafeFuncs[i]))
		{
			DevMsg(VScript_GetNativeLogContext(nContext), "Replacing %s in %s\n", pFuncReg->squirrelFuncName, VScript_GetContextAsString(nContext));
			pFuncReg->funcPtr = Script_StubbedFunc;
		}
	}

	if (nContext == ScriptContext::SERVER)
		return o_CSquirrelVM__RegisterFunctionServer(sqvm, pFuncReg, unknown);
	else
		return o_CSquirrelVM__RegisterFunctionClient(sqvm, pFuncReg, unknown);
}

bool h_CSquirrelVM__CallScriptInitCallback(CSquirrelVM* sqvm, const char* callback)
{
	ScriptContext nContext = (ScriptContext)sqvm->vmContext;
	bool bShouldCallCustomCallbacks = true;

	bool (*o_CSquirrelVM__CallScriptInitCallback)(CSquirrelVM * sqvm, const char* pszCallback);
	if (nContext == ScriptContext::SERVER)
		o_CSquirrelVM__CallScriptInitCallback = o_CSquirrelVM__CallScriptInitCallbackServer;
	else
		o_CSquirrelVM__CallScriptInitCallback = o_CSquirrelVM__CallScriptInitCallbackClient;

	if (nContext == ScriptContext::CLIENT)
	{
		if (!strcmp(callback, "UICodeCallback_UIInit"))
			nContext = ScriptContext::UI;
		else if (strcmp(callback, "ClientCodeCallback_MapSpawn"))
			bShouldCallCustomCallbacks = false;
	}
	else if (nContext == ScriptContext::SERVER)
		bShouldCallCustomCallbacks = !strcmp(callback, "CodeCallback_MapSpawn");

	if (bShouldCallCustomCallbacks)
	{
		for (Mod mod : g_pModManager->m_LoadedMods)
		{
			if (!mod.m_bEnabled)
				continue;

			for (ModScript script : mod.Scripts)
			{
				for (ModScriptCallback modCallback : script.Callbacks)
				{
					if (modCallback.Context == nContext && modCallback.BeforeCallback.length())
					{
						DevMsg(VScript_GetNativeLogContext(nContext), "Running custom %s script callback \"%s\"\n", VScript_GetContextAsString(nContext), modCallback.BeforeCallback.c_str());
						o_CSquirrelVM__CallScriptInitCallback(sqvm, modCallback.BeforeCallback.c_str());
					}
				}
			}
		}
	}

	DevMsg(VScript_GetNativeLogContext(nContext), "%s CodeCallback %s called\n", VScript_GetContextAsString(nContext), callback);
	if (!bShouldCallCustomCallbacks)
		DevMsg(VScript_GetNativeLogContext(nContext), "Not executing custom callbacks for CodeCallback %s\n", callback);
	bool ret = o_CSquirrelVM__CallScriptInitCallback(sqvm, callback);

	// run after callbacks
	if (bShouldCallCustomCallbacks)
	{
		for (Mod mod : g_pModManager->m_LoadedMods)
		{
			if (!mod.m_bEnabled)
				continue;

			for (ModScript script : mod.Scripts)
			{
				for (ModScriptCallback modCallback : script.Callbacks)
				{
					if (modCallback.Context == nContext && modCallback.AfterCallback.length())
					{
						DevMsg(VScript_GetNativeLogContext(nContext), "Running custom %s script callback \"%s\"\n", VScript_GetContextAsString(nContext), modCallback.AfterCallback.c_str());
						o_CSquirrelVM__CallScriptInitCallback(sqvm, modCallback.AfterCallback.c_str());
					}
				}
			}
		}
	}

	return ret;
}

ON_DLL_LOAD("client.dll", SquirrelReVSquirrelClient, (CModule module))
{
	o_CSquirrelVM__InitClient = module.Offset(0xE3B0).RCast<bool (*)(CSquirrelVM*, ScriptContext, float)>();
	HookAttach(&(PVOID&)o_CSquirrelVM__InitClient, (PVOID)h_CSquirrelVM__InitClient);

	o_CSquirrelVM__RegisterFunctionClient = module.Offset(0x108E0).RCast<int64_t (*)(CSquirrelVM*, SQFuncRegistration*, char)>();
	HookAttach(&(PVOID&)o_CSquirrelVM__RegisterFunctionClient, (PVOID)h_CSquirrelVM__RegisterFunction);

	o_CSquirrelVM__CallScriptInitCallbackClient = module.Offset(0x10190).RCast<bool (*)(CSquirrelVM * sqvm, const char* pszCallback)>();
	HookAttach(&(PVOID&)o_CSquirrelVM__CallScriptInitCallbackClient, (PVOID)h_CSquirrelVM__CallScriptInitCallback);

	o_CSquirrelVM__DefConstClient = module.Offset(0x12120).RCast<void (*)(CSquirrelVM*, const SQChar*, int)>();
	o_CSquirrelVM__CompileFileClient = module.Offset(0xF950).RCast<bool (*)(CSquirrelVM*, const char*, const char*, int)>();

	o_CSquirrelVM__GetEntityInstanceClient = module.Offset(0x114F0).RCast<void* (*)(CSquirrelVM*, SQObject*, char**)>();
	o_CSquirrelVM__GetEntityConstantClient = module.Offset(0x3E49B0).RCast<char** (*)()>();
}

ON_DLL_LOAD("server.dll", SquirrelReVSquirrelServer, (CModule module))
{
	o_CSquirrelVM__InitServer = module.Offset(0x1B7E0).RCast<bool (*)(CSquirrelVM*, ScriptContext, float)>();
	HookAttach(&(PVOID&)o_CSquirrelVM__InitServer, (PVOID)h_CSquirrelVM__InitServer);

	o_CSquirrelVM__RegisterFunctionServer = module.Offset(0x1DD10).RCast<int64_t (*)(CSquirrelVM*, SQFuncRegistration*, char)>();
	HookAttach(&(PVOID&)o_CSquirrelVM__RegisterFunctionServer, (PVOID)h_CSquirrelVM__RegisterFunction);

	o_CSquirrelVM__CallScriptInitCallbackServer = module.Offset(0x1D5C0).RCast<bool (*)(CSquirrelVM * sqvm, const char* pszCallback)>();
	HookAttach(&(PVOID&)o_CSquirrelVM__CallScriptInitCallbackServer, (PVOID)h_CSquirrelVM__CallScriptInitCallback);

	o_CSquirrelVM__DefConstServer = module.Offset(0x1F550).RCast<void (*)(CSquirrelVM*, const SQChar*, int)>();
	o_CSquirrelVM__CompileFileServer = module.Offset(0x1CD80).RCast<bool (*)(CSquirrelVM*, const char*, const char*, int)>();

	o_CSquirrelVM__GetEntityInstanceServer = module.Offset(0x1E920).RCast<void* (*)(CSquirrelVM*, SQObject*, char**)>();
	o_CSquirrelVM__GetEntityConstantServer = module.Offset(0x418AF0).RCast<char** (*)()>();
}
