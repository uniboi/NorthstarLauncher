#include "mods/modmanager.h"
#include "shared/misccommands.h"
#include "engine/host.h"

void (*o_Host_Init)(bool bDedicated);

void h_Host_Init(bool bDedicated)
{
	DevMsg(eLog::ENGINE, "Host_Init(bDedicated: %d)\n", bDedicated);
	o_Host_Init(bDedicated);
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

bool (*o_Host_NewGame)(char* pszMapName, bool bLoadGame, bool bBackgroundLevel, bool bSplitScreenConnect, const char* pszOldMap, const char* pszLandmark);

bool h_Host_NewGame(char* pszMapName, bool bLoadGame, bool bBackgroundLevel, bool bSplitScreenConnect, const char* pszOldMap, const char* pszLandmark)
{
	return o_Host_NewGame(pszMapName, bLoadGame, bBackgroundLevel, bSplitScreenConnect, pszOldMap, pszLandmark);
}

ON_DLL_LOAD("engine.dll", Host_Init, (CModule module))
{
	o_Host_Init = module.Offset(0x155EA0).RCast<void (*)(bool)>();
	HookAttach(&(PVOID&)o_Host_Init, (PVOID)h_Host_Init);

	o_Host_NewGame = module.Offset(0x156B10).RCast<bool (*)(char*, bool, bool, bool, const char*, const char*)>();
	HookAttach(&(PVOID&)o_Host_NewGame, (PVOID)h_Host_NewGame);

	g_pEngineParms = module.Offset(0x13159310).RCast<EngineParms_t*>();
}
