#include "tier1/convar.h"
#include "GameConsole.h"
#include "tier1/cmd.h"

CGameConsole* g_pGameConsole = nullptr;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_CGameConsole_OnCommandSubmitted)(void /*CConsoleDialog*/* pConsoleDialog, const char* pszCommand);

void h_CGameConsole_OnCommandSubmitted(void /*CConsoleDialog*/* pConsoleDialog, const char* pszCommand)
{
	DevMsg(eLog::NONE, "] %s\n", pszCommand);
	o_CGameConsole_OnCommandSubmitted(pConsoleDialog, pszCommand);
}

ON_DLL_LOAD_CLIENT("client.dll", SourceConsole, (CModule module))
{
	o_CGameConsole_OnCommandSubmitted = module.Offset(0x4A2550).RCast<void (*)(void*, const char*)>();
	HookAttach(&(PVOID&)o_CGameConsole_OnCommandSubmitted, (PVOID)h_CGameConsole_OnCommandSubmitted);

	g_pGameConsole = Sys_GetFactoryPtr("client.dll", "GameConsole004").RCast<CGameConsole*>();
}
