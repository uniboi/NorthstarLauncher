#include "tier1/convar.h"
#include "GameConsole.h"
#include "tier1/cmd.h"

AUTOHOOK_INIT()

CGameConsole* g_pGameConsole = nullptr;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
// clang-format off
AUTOHOOK(CGameConsole_OnCommandSubmitted, client.dll + 0x4A2550, void, __fastcall,
	(void /*CConsoleDialog*/* pConsoleDialog, const char* pszCommand))
// clang-format on
{
	DevMsg(eLog::NONE, "] %s\n", pszCommand);
	CGameConsole_OnCommandSubmitted(pConsoleDialog, pszCommand);
}

ON_DLL_LOAD_CLIENT_RELIESON("client.dll", SourceConsole, ConCommand, (CModule module))
{
	AUTOHOOK_DISPATCH()

	g_pGameConsole = Sys_GetFactoryPtr("client.dll", "GameConsole004").RCast<CGameConsole*>();
}
