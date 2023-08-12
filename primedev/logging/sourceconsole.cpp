#include "core/convar/convar.h"
#include "sourceconsole.h"
#include "core/convar/concommand.h"
#include "util/printcommands.h"
#include "tier1/interface.h"

AUTOHOOK_INIT()

CGameConsole* g_pGameConsole = nullptr;

void ConCommand_toggleconsole(const CCommand& arg)
{
	if (g_pGameConsole->IsConsoleVisible())
		g_pGameConsole->Hide();
	else
		g_pGameConsole->Activate();
}

void ConCommand_showconsole(const CCommand& arg)
{
	g_pGameConsole->Activate();
}

void ConCommand_hideconsole(const CCommand& arg)
{
	g_pGameConsole->Hide();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
// clang-format off
AUTOHOOK(CGameConsole_OnCommandSubmitted, client.dll + 0x4A2550, void, __fastcall,
	(CConsoleDialog* pConsoleDialog, const char* pszCommand))
// clang-format on
{
	DevMsg(eLog::NONE, "] %s\n", pszCommand);

	TryPrintCvarHelpForCommand(pszCommand);

	CGameConsole_OnCommandSubmitted(pConsoleDialog, pszCommand);
}

// called from sourceinterface.cpp in client createinterface hooks, on GameClientExports001
void InitialiseConsoleOnInterfaceCreation()
{
	g_pGameConsole->Initialize();
}

ON_DLL_LOAD_CLIENT_RELIESON("client.dll", SourceConsole, ConCommand, (CModule module))
{
	AUTOHOOK_DISPATCH()

	g_pGameConsole = Sys_GetFactoryPtr("client.dll", "GameConsole004").RCast<CGameConsole*>();

	RegisterConCommand("toggleconsole", ConCommand_toggleconsole, "Show/hide the console.", FCVAR_DONTRECORD);
	RegisterConCommand("showconsole", ConCommand_showconsole, "Show the console.", FCVAR_DONTRECORD);
	RegisterConCommand("hideconsole", ConCommand_hideconsole, "Hide the console.", FCVAR_DONTRECORD);
}
