#include "engine/hoststate.h"
#include "shared/playlist.h"
#include "engine/server/server.h"
#include "shared/exploit_fixes/ns_limits.h"
#include "networksystem/atlas.h"
#include "engine/edict.h"
#include "originsdk/origin.h"
#include "tier0/taskscheduler.h"

#include "vscript/vscript.h"

CHostState* g_pHostState;

std::string sLastMode;

void (*_Cmd_Exec_f)(const CCommand& arg, bool bOnlyIfExists, bool bUseWhitelists);

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void ServerStartingOrChangingMap()
{
	// directly call _Cmd_Exec_f to avoid weirdness with ; being in mp_gamemode potentially
	// if we ran exec {mp_gamemode} and mp_gamemode contained semicolons, this could be used to execute more commands
	char* commandBuf[1040]; // assumedly this is the size of CCommand since we don't have an actual constructor
	memset(commandBuf, 0, sizeof(commandBuf));
	CCommand tempCommand = *(CCommand*)&commandBuf;
	if (sLastMode.length() && CCommand__Tokenize(tempCommand, fmt::format("exec server/cleanup_gamemode_{}", sLastMode).c_str(), cmd_source_t::kCommandSrcCode))
		_Cmd_Exec_f(tempCommand, false, false);

	memset(commandBuf, 0, sizeof(commandBuf));
	if (CCommand__Tokenize(tempCommand, fmt::format("exec server/setup_gamemode_{}", sLastMode = Cvar_mp_gamemode->GetString()).c_str(), cmd_source_t::kCommandSrcCode))
	{
		_Cmd_Exec_f(tempCommand, false, false);
	}

	Cbuf_Execute(); // exec everything right now

	// net_data_block_enabled is required for sp, force it if we're on an sp map
	// sucks for security but just how it be
	if (!strncmp(g_pHostState->m_levelName, "sp_", 3))
	{
		g_pCVar->FindVar("net_data_block_enabled")->SetValue(true);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_CHostState__State_NewGame)(CHostState* self);

void h_CHostState__State_NewGame(CHostState* self)
{
	DevMsg(eLog::ENGINE, "HostState: NewGame\n");

	Cbuf_AddText(Cbuf_GetCurrentPlayer(), "exec autoexec_ns_server", cmd_source_t::kCommandSrcCode);
	Cbuf_Execute();

	// need to do this to ensure we don't go to private match
	// if (g_pServerAuthentication->m_bNeedLocalAuthForNewgame)
	//	SetCurrentPlaylist("tdm");

	ServerStartingOrChangingMap();

	double dStartTime = Plat_FloatTime();
	o_CHostState__State_NewGame(self);
	DevMsg(eLog::ENGINE, "loading took %f sec\n", Plat_FloatTime() - dStartTime);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_CHostState__State_LoadGame)(CHostState* self);

void h_CHostState__State_LoadGame(CHostState* self)
{
	// singleplayer server starting
	// useless in 99% of cases but without it things could potentially break very much

	DevMsg(eLog::ENGINE, "HostState: LoadGame\n");

	Cbuf_AddText(Cbuf_GetCurrentPlayer(), "exec autoexec_ns_server", cmd_source_t::kCommandSrcCode);
	Cbuf_Execute();

	// this is normally done in ServerStartingOrChangingMap(), but seemingly the map name isn't set at this point
	g_pCVar->FindVar("net_data_block_enabled")->SetValue(true);
	// g_pServerAuthentication->m_bStartingLocalSPGame = true;

	double dStartTime = Plat_FloatTime();
	o_CHostState__State_LoadGame(self);
	DevMsg(eLog::ENGINE, "loading took %f sec\n", Plat_FloatTime() - dStartTime);

	// no server presence, can't do it because no map name in hoststate
	// and also not super important for sp saves really

	// g_pServerAuthentication->m_bNeedLocalAuthForNewgame = false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_CHostState__State_ChangeLevelMP)(CHostState* self);

void h_CHostState__State_ChangeLevelMP(CHostState* self)
{
	DevMsg(eLog::ENGINE, "HostState: ChangeLevelMP\n");

	ServerStartingOrChangingMap();

	double dStartTime = Plat_FloatTime();
	o_CHostState__State_ChangeLevelMP(self);
	DevMsg(eLog::ENGINE, "loading took %f sec\n", Plat_FloatTime() - dStartTime);

	// g_pServerPresence->SetMap(g_pHostState->m_levelName);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_CHostState__State_GameShutdown)(CHostState* self);

void h_CHostState__State_GameShutdown(CHostState* self)
{
	DevMsg(eLog::ENGINE, "HostState: GameShutdown\n");

	// g_pServerPresence->DestroyPresence();

	o_CHostState__State_GameShutdown(self);

	// run gamemode cleanup cfg now instead of when we start next map
	if (sLastMode.length())
	{
		char* commandBuf[1040]; // assumedly this is the size of CCommand since we don't have an actual constructor
		memset(commandBuf, 0, sizeof(commandBuf));
		CCommand tempCommand = *(CCommand*)&commandBuf;
		if (CCommand__Tokenize(tempCommand, fmt::format("exec server/cleanup_gamemode_{}", sLastMode).c_str(), cmd_source_t::kCommandSrcCode))
		{
			_Cmd_Exec_f(tempCommand, false, false);
			Cbuf_Execute();
		}

		sLastMode.clear();
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void (*o_CHostState__FrameUpdate)(CHostState* self, double flCurrentTime, float flFrameTime);

void h_CHostState__FrameUpdate(CHostState* self, double flCurrentTime, float flFrameTime)
{
	o_CHostState__FrameUpdate(self, flCurrentTime, flFrameTime);

	if (g_pServer->IsActive())
	{
		// Only bother with reporting to atas in MP
		if (g_pServerGlobalVariables->m_nGameMode == MP_MODE)
		{
			g_pAtlasServer->HeartBeat(flCurrentTime);
		}

		// update limits for frame
		g_pServerLimits->RunFrame(flCurrentTime, flFrameTime);
	}
	else
	{
		// This checks if we're registered so calling each frame is fine
		g_pAtlasServer->UnregisterSelf();
	}

	g_pTaskScheduler->RunFrame();
}

//-----------------------------------------------------------------------------
// Purpose: Gets called when loading a map from the main menu
//-----------------------------------------------------------------------------
void (*o_HostState_NewGame)(char const* pMapName, bool bBackground, bool bSplitScreenConnect);

void h_HostState_NewGame(char const* pMapName, bool bBackground, bool bSplitScreenConnect)
{
	// Make sure auth data is cleared before first map load
	g_pAtlasServer->ClearAuthInfo();

	// Get auth data for self only on listen server, in mp when loading from main menu
	if (!IsDedicatedServer() && !strncmp(pMapName, "mp_", 3))
	{
		g_pAtlasServer->AuthenticateLocalClient(g_pLocalPlayerUserID);
	}

	o_HostState_NewGame(pMapName, bBackground, bSplitScreenConnect);
}

ON_DLL_LOAD("engine.dll", HostState, (CModule module))
{
	o_CHostState__State_NewGame = module.Offset(0x16E7D0).RCast<void (*)(CHostState*)>();
	HookAttach(&(PVOID&)o_CHostState__State_NewGame, (PVOID)h_CHostState__State_NewGame);

	o_CHostState__State_LoadGame = module.Offset(0x16E730).RCast<void (*)(CHostState*)>();
	HookAttach(&(PVOID&)o_CHostState__State_LoadGame, (PVOID)h_CHostState__State_LoadGame);

	o_CHostState__State_ChangeLevelMP = module.Offset(0x16E520).RCast<void (*)(CHostState*)>();
	HookAttach(&(PVOID&)o_CHostState__State_ChangeLevelMP, (PVOID)h_CHostState__State_ChangeLevelMP);

	o_CHostState__State_GameShutdown = module.Offset(0x16E640).RCast<void (*)(CHostState*)>();
	HookAttach(&(PVOID&)o_CHostState__State_GameShutdown, (PVOID)h_CHostState__State_GameShutdown);

	o_CHostState__FrameUpdate = module.Offset(0x16DB00).RCast<void (*)(CHostState*, double, float)>();
	HookAttach(&(PVOID&)o_CHostState__FrameUpdate, (PVOID)h_CHostState__FrameUpdate);

	o_HostState_NewGame = module.Offset(0x16DF90).RCast<void (*)(char const*, bool, bool)>();
	HookAttach(&(PVOID&)o_HostState_NewGame, (PVOID)h_HostState_NewGame);

	_Cmd_Exec_f = module.Offset(0x1232C0).RCast<void (*)(const CCommand&, bool, bool)>();

	g_pHostState = module.Offset(0x7CF180).RCast<CHostState*>();
}
