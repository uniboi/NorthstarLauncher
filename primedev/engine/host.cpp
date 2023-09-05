#include "mods/modmanager.h"
#include "shared/misccommands.h"
#include "engine/host.h"

AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(Host_Init, engine.dll + 0x155EA0,
void, __fastcall, (bool bDedicated))
// clang-format on
{
	DevMsg(eLog::ENGINE, "Host_Init(bDedicated: %d)\n", bDedicated);
	Host_Init(bDedicated);
	FixupCvarFlags();

	// Hardcoded mod functionality, add something to mod.json or just teach users on how to +exec as a replacement ot this
	Warning(eLog::ENGINE, "'autoexec_ns_*' files are deprecated!\n");
	// client/server autoexecs on necessary platforms
	// dedi needs autoexec_ns_server on boot, while non-dedi will run it on on listen server start
	if (bDedicated)
		Cbuf_AddText(Cbuf_GetCurrentPlayer(), "exec autoexec_ns_server", cmd_source_t::kCommandSrcCode);
	else
		Cbuf_AddText(Cbuf_GetCurrentPlayer(), "exec autoexec_ns_client", cmd_source_t::kCommandSrcCode);
}

AUTOHOOK(Host_NewGame, engine.dll + 0x156B10, bool, __fastcall, (char* pszMapName, bool bLoadGame, bool bBackgroundLevel, bool bSplitScreenConnect, const char* pszOldMap, const char* pszLandmark))
{
	return Host_NewGame(pszMapName, bLoadGame, bBackgroundLevel, bSplitScreenConnect, pszOldMap, pszLandmark);
}

ON_DLL_LOAD("engine.dll", Host_Init, (CModule module))
{
	AUTOHOOK_DISPATCH()

	g_pEngineParms = module.Offset(0x13159310).RCast<EngineParms_t*>();
}
