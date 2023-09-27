#include "vscript/vscript.h"

#include "game/client/vscript_client.h"
#include "game/server/vscript_server.h"
#include "game/shared/vscript_shared.h"
#include "mods/modsavefiles.h"

//-----------------------------------------------------------------------------
// Purpose: Returns vscript context from string
// Input  : svContext -
// Output :
//-----------------------------------------------------------------------------
ScriptContext VScript_GetContextFromString(std::string& svContext)
{
	if (svContext == "UI")
		return ScriptContext::UI;
	if (svContext == "CLIENT")
		return ScriptContext::CLIENT;
	if (svContext == "SERVER")
		return ScriptContext::SERVER;
	else
		return ScriptContext::INVALID;
}

//-----------------------------------------------------------------------------
// Purpose: Returns vscript context as a string
// Input  : nContext -
// Output :
//-----------------------------------------------------------------------------
const char* VScript_GetContextAsString(ScriptContext nContext)
{
	switch (nContext)
	{
	case ScriptContext::CLIENT:
		return "CLIENT";
	case ScriptContext::SERVER:
		return "SERVER";
	case ScriptContext::UI:
		return "UI";
	default:
		return "UNKNOWN";
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns script log context based on Script context
// Input  : nContext -
// Output :
//-----------------------------------------------------------------------------
eLog VScript_GetScriptLogContext(ScriptContext nContext)
{
	switch (nContext)
	{
	case ScriptContext::SERVER:
		return eLog::SCRIPT_SERVER;
	case ScriptContext::CLIENT:
		return eLog::SCRIPT_CLIENT;
	case ScriptContext::UI:
		return eLog::SCRIPT_UI;
	}

	return eLog::NONE;
}

//-----------------------------------------------------------------------------
// Purpose:Returns native log context based on Script context
// Input  : nContext -
// Output :
//-----------------------------------------------------------------------------
eLog VScript_GetNativeLogContext(ScriptContext nContext)
{
	switch (nContext)
	{
	case ScriptContext::SERVER:
		return eLog::SERVER;
	case ScriptContext::CLIENT:
		return eLog::CLIENT;
	case ScriptContext::UI:
		return eLog::UI;
	}

	return eLog::NONE;
}

// TODO [Fifty]: Respawn stripped CScriptManager, the self pointer is a singleton so figure out what that is

CSquirrelVM* (*o_CScriptManager__CreateNewClientVM)(void* a1, ScriptContext nContext);

CSquirrelVM* h_CScriptManager__CreateNewClientVM(void* a1, ScriptContext nContext)
{
	// Setup globals
	CSquirrelVM* sqvm = o_CScriptManager__CreateNewClientVM(a1, nContext);
	if (nContext == ScriptContext::CLIENT)
	{
		g_pClientVM = sqvm;
		VScript_RegisterClientFunctions(sqvm);
	}
	else
	{
		g_pUIVM = sqvm;
		VScript_RegisterUIFunctions(sqvm);
	}

	VScript_RegisterSharedFunctions(sqvm);

	// Setup constants
	for (auto& pair : g_pModManager->m_DependencyConstants)
	{
		bool bWasFound = false;

		for (Mod& dependency : g_pModManager->m_LoadedMods)
		{
			if (!dependency.m_bEnabled)
				continue;

			if (dependency.Name == pair.second)
			{
				bWasFound = true;
				break;
			}
		}
		sqvm->AddConstant(pair.first.c_str(), bWasFound);
	}
	sqvm->AddConstant("MAX_FOLDER_SIZE", GetMaxSaveFolderSize() / 1024);

	DevMsg(VScript_GetNativeLogContext(nContext), "CreateNewVM %s %p\n", VScript_GetContextAsString(nContext), (void*)sqvm);
	return sqvm;
}

CSquirrelVM* (*o_CScriptManager__CreateNewServerVM)(void* a1, ScriptContext nContext);

CSquirrelVM* h_CScriptManager__CreateNewServerVM(void* a1, ScriptContext nContext)
{
	// Setup globals
	CSquirrelVM* sqvm = o_CScriptManager__CreateNewServerVM(a1, nContext);

	g_pServerVM = sqvm;
	VScript_RegisterServerFunctions(sqvm);
	VScript_RegisterSharedFunctions(sqvm);

	// Setup constants
	for (auto& pair : g_pModManager->m_DependencyConstants)
	{
		bool bWasFound = false;

		for (Mod& dependency : g_pModManager->m_LoadedMods)
		{
			if (!dependency.m_bEnabled)
				continue;

			if (dependency.Name == pair.second)
			{
				bWasFound = true;
				break;
			}
		}
		sqvm->AddConstant(pair.first.c_str(), bWasFound);
	}
	sqvm->AddConstant("MAX_FOLDER_SIZE", GetMaxSaveFolderSize() / 1024);

	DevMsg(VScript_GetNativeLogContext(nContext), "CreateNewVM %s %p\n", VScript_GetContextAsString(nContext), (void*)sqvm);
	return sqvm;
}

void (*o_CScriptManager__DestroyClientVM)(void* a1, CSquirrelVM* sqvm);
void h_CScriptManager__DestroyClientVM(void* a1, CSquirrelVM* sqvm)
{
	ScriptContext nContext = (ScriptContext)sqvm->vmContext;

	sqvm->CallDestroyCallbacks();

	if (nContext == ScriptContext::CLIENT)
	{
		g_pClientVM = nullptr;
	}
	else
	{
		g_pUIVM = nullptr;
	}

	DevMsg(VScript_GetNativeLogContext(nContext), "DestroyVM %s %p\n", VScript_GetContextAsString(nContext), (void*)sqvm);

	o_CScriptManager__DestroyClientVM(a1, sqvm);
}

void (*o_CScriptManager__DestroyServerVM)(void* a1, CSquirrelVM* sqvm);
void h_CScriptManager__DestroyServerVM(void* a1, CSquirrelVM* sqvm)
{
	sqvm->CallDestroyCallbacks();

	g_pServerVM = nullptr;

	DevMsg(VScript_GetNativeLogContext(ScriptContext::SERVER), "DestroyVM %s %p\n", VScript_GetContextAsString(ScriptContext::SERVER), (void*)sqvm);

	o_CScriptManager__DestroyServerVM(a1, sqvm);
}

ON_DLL_LOAD("client.dll", VscriptClient, (CModule module))
{
	o_CScriptManager__CreateNewClientVM = module.Offset(0x26130).RCast<CSquirrelVM* (*)(void*, ScriptContext)>();
	HookAttach(&(PVOID&)o_CScriptManager__CreateNewClientVM, (PVOID)h_CScriptManager__CreateNewClientVM);

	o_CScriptManager__DestroyClientVM = module.Offset(0x26E70).RCast<void (*)(void*, CSquirrelVM*)>();
	HookAttach(&(PVOID&)o_CScriptManager__DestroyClientVM, (PVOID)h_CScriptManager__DestroyClientVM);
}

ON_DLL_LOAD("server.dll", VScriptServer, (CModule module))
{
	o_CScriptManager__CreateNewServerVM = module.Offset(0x260E0).RCast<CSquirrelVM* (*)(void*, ScriptContext)>();
	HookAttach(&(PVOID&)o_CScriptManager__CreateNewServerVM, (PVOID)h_CScriptManager__CreateNewServerVM);

	o_CScriptManager__DestroyServerVM = module.Offset(0x26E20).RCast<void (*)(void*, CSquirrelVM*)>();
	HookAttach(&(PVOID&)o_CScriptManager__DestroyServerVM, (PVOID)h_CScriptManager__DestroyServerVM);
}
