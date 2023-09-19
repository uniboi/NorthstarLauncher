#include "common/callbacks.h"

#include "engine/cdll_engine_int.h"
#include "client/localchatwriter.h"
#include "gameui/GameConsole.h"
#include "mods/modmanager.h"
#include "squirrel/squirrel.h"
#include "networksystem/bansystem.h"
#include "engine/client/client.h"
#include "engine/server/server.h"
#include "engine/hoststate.h"
#include "originsdk/origin.h"
#include "shared/playlist.h"
#include "engine/datamap.h"
#include "game/server/gameinterface.h"
#include "engine/vengineserver_impl.h"
#include "rtech/datatable.h"
#include "networksystem/atlas.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerName_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	// update engine hostname cvar
	Cvar_hostname->SetValue(Cvar_ns_server_name->GetString());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerDesc_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	//
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerPass_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	//
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_dump_datamap(const CCommand& args)
{
	HMODULE hServer = GetModuleHandleA("server.dll");
	if (!hServer)
	{
		Error(eLog::ENGINE, NO_ERROR, "server.dll not loaded!\n");
	}

	CModule mServer(hServer);

	// CTriggerHurt: 0xB2DE80
	// CPlayer: 0xB86460
	// CFuncBrush: 0xB145C0
	datamap_t* pMap = mServer.Offset(0xB86460).RCast<datamap_t*>();

	DataMap_Dump(pMap);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_Say_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
		CEngineClient__Saytext(nullptr, args.ArgS(), true, false);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_SayTeam_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
		CEngineClient__Saytext(nullptr, args.ArgS(), true, true);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_Log_f(const CCommand& args)
{
	if (args.ArgC() >= 2)
	{
		LocalChatWriter(LocalChatWriter::GameContext).WriteLine(args.ArgS());
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ToggleConsole_f(const CCommand& args)
{
	if (g_pGameConsole->IsConsoleVisible())
		g_pGameConsole->Hide();
	else
		g_pGameConsole->Activate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ShowConsole_f(const CCommand& args)
{
	g_pGameConsole->Activate();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_HideConsole_f(const CCommand& args)
{
	g_pGameConsole->Hide();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_reload_mods_f(const CCommand& args)
{
	g_pModManager->LoadMods();
}
//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ns_fetchservers_f(const CCommand& args)
{
	//
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ns_script_servertoclientstringcommand_f(const CCommand& arg)
{
	if (g_pSquirrel<ScriptContext::CLIENT>->m_pSQVM)
		g_pSquirrel<ScriptContext::CLIENT>->Call("NSClientCodeCallback_RecievedServerToClientStringCommand", arg.ArgS());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_dump_datatable_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		DevMsg(eLog::NS, "usage: dump_datatable datatable/tablename.rpak\n");
		return;
	}

	DumpDatatable(args.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_dump_datatables_f(const CCommand& args)
{
	// likely not a comprehensive list, might be missing a couple?
	static const std::vector<const char*> VANILLA_DATATABLE_PATHS = {"datatable/burn_meter_rewards.rpak",
																	 "datatable/burn_meter_store.rpak",
																	 "datatable/calling_cards.rpak",
																	 "datatable/callsign_icons.rpak",
																	 "datatable/camo_skins.rpak",
																	 "datatable/default_pilot_loadouts.rpak",
																	 "datatable/default_titan_loadouts.rpak",
																	 "datatable/faction_leaders.rpak",
																	 "datatable/fd_awards.rpak",
																	 "datatable/features_mp.rpak",
																	 "datatable/non_loadout_weapons.rpak",
																	 "datatable/pilot_abilities.rpak",
																	 "datatable/pilot_executions.rpak",
																	 "datatable/pilot_passives.rpak",
																	 "datatable/pilot_properties.rpak",
																	 "datatable/pilot_weapons.rpak",
																	 "datatable/pilot_weapon_features.rpak",
																	 "datatable/pilot_weapon_mods.rpak",
																	 "datatable/pilot_weapon_mods_common.rpak",
																	 "datatable/playlist_items.rpak",
																	 "datatable/titans_mp.rpak",
																	 "datatable/titan_abilities.rpak",
																	 "datatable/titan_executions.rpak",
																	 "datatable/titan_fd_upgrades.rpak",
																	 "datatable/titan_nose_art.rpak",
																	 "datatable/titan_passives.rpak",
																	 "datatable/titan_primary_mods.rpak",
																	 "datatable/titan_primary_mods_common.rpak",
																	 "datatable/titan_primary_weapons.rpak",
																	 "datatable/titan_properties.rpak",
																	 "datatable/titan_skins.rpak",
																	 "datatable/titan_voices.rpak",
																	 "datatable/unlocks_faction_level.rpak",
																	 "datatable/unlocks_fd_titan_level.rpak",
																	 "datatable/unlocks_player_level.rpak",
																	 "datatable/unlocks_random.rpak",
																	 "datatable/unlocks_titan_level.rpak",
																	 "datatable/unlocks_weapon_level_pilot.rpak",
																	 "datatable/weapon_skins.rpak",
																	 "datatable/xp_per_faction_level.rpak",
																	 "datatable/xp_per_fd_titan_level.rpak",
																	 "datatable/xp_per_player_level.rpak",
																	 "datatable/xp_per_titan_level.rpak",
																	 "datatable/xp_per_weapon_level.rpak",
																	 "datatable/faction_leaders_dropship_anims.rpak",
																	 "datatable/score_events.rpak",
																	 "datatable/startpoints.rpak",
																	 "datatable/sp_levels.rpak",
																	 "datatable/community_entries.rpak",
																	 "datatable/spotlight_images.rpak",
																	 "datatable/death_hints_mp.rpak",
																	 "datatable/flightpath_assets.rpak",
																	 "datatable/earn_meter_mp.rpak",
																	 "datatable/battle_chatter_voices.rpak",
																	 "datatable/battle_chatter.rpak",
																	 "datatable/titan_os_conversations.rpak",
																	 "datatable/faction_dialogue.rpak",
																	 "datatable/grunt_chatter_mp.rpak",
																	 "datatable/spectre_chatter_mp.rpak",
																	 "datatable/pain_death_sounds.rpak",
																	 "datatable/caller_ids_mp.rpak"};

	for (const char* datatable : VANILLA_DATATABLE_PATHS)
		DumpDatatable(datatable);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ban_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* player = g_pServer->GetClient(i);

		if (!strcmp(player->m_szServerName, args.Arg(1)) || !strcmp(player->m_UID, args.Arg(1)))
		{
			g_pBanSystem->BanUID(strtoull(player->m_UID, nullptr, 10));
			CClient__Disconnect(player, 1, "Banned from server");
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_unban_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	// assumedly the player being unbanned here wasn't already connected, so don't need to iterate over players or anything
	g_pBanSystem->UnbanUID(strtoull(args.Arg(1), nullptr, 10));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_clearbanlist_f(const CCommand& args)
{
	g_pBanSystem->ClearBanlist();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_playlist_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	SetCurrentPlaylist(args.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_setplaylistvaroverride_f(const CCommand& args)
{
	if (args.ArgC() < 3)
		return;

	for (int i = 1; i < args.ArgC(); i += 2)
		SetPlaylistVarOverride(args.Arg(i), args.Arg(i + 1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_script_ui_f(const CCommand& args)
{
	g_pSquirrel<ScriptContext::UI>->ExecuteCode(args.ArgS());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_script_cl_f(const CCommand& args)
{
	g_pSquirrel<ScriptContext::CLIENT>->ExecuteCode(args.ArgS());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_script_sv_f(const CCommand& args)
{
	g_pSquirrel<ScriptContext::SERVER>->ExecuteCode(args.ArgS());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_find_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: find <string> [<string>...]\n");
		return;
	}

	CCVarIteratorInternal* pIter = g_pCVar->FactoryInternalIterator();

	char szLowerName[256];
	char szLowerTerm[256];

	// Loop through all commandbases
	for (pIter->SetFirst(); pIter->IsValid(); pIter->Next())
	{
		ConCommandBase* pCmdBase = pIter->Get();
		bool bFound = false;

		// Loop through all passed arguments
		for (int i = 0; i < arg.ArgC() - 1; i++)
		{
			strncpy_s(szLowerName, sizeof(szLowerName), pCmdBase->m_pszName, sizeof(szLowerName) - 1);
			strncpy_s(szLowerTerm, sizeof(szLowerTerm), arg.Arg(i + 1), sizeof(szLowerTerm) - 1);

			for (int i = 0; szLowerName[i]; i++)
				szLowerName[i] = tolower(szLowerName[i]);

			for (int i = 0; szLowerTerm[i]; i++)
				szLowerTerm[i] = tolower(szLowerTerm[i]);

			if (strstr(szLowerName, szLowerTerm))
			{
				bFound = true;
				break;
			}
		}

		if (bFound)
		{
			g_pCVar->PrintHelpString(pCmdBase->m_pszName);
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_help_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: help <string>\n");
		return;
	}

	g_pCVar->PrintHelpString(arg.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_printcvaraddr_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: cvar_printaddr <string>\n");
		return;
	}

	const ConCommandBase* pCmdBase = g_pCVar->FindCommandBase(arg.Arg(1));

	if (!pCmdBase)
	{
		Error(eLog::NS, NO_ERROR, "'%s' doesn't exist!\n", arg.Arg(1));
		return;
	}

	LogPtrAdr(pCmdBase->m_pszName, (void*)pCmdBase);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_maps_f(const CCommand& args)
{
	if (args.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: maps <substring>\n");
		DevMsg(eLog::NS, "maps * for full listing\n");
		return;
	}

	for (MapVPKInfo_t& map : g_pModManager->m_vMapList)
	{
		if ((*args.Arg(1) == '*' && !args.Arg(1)[1]) || strstr(map.svName.c_str(), args.Arg(1)))
		{
			DevMsg(eLog::NS, "(%i) %s\n", map.eSource, map.svName.c_str());
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_CreateFakePlayer_f(const CCommand& args)
{
	if (args.ArgC() < 3)
	{
		DevMsg(eLog::NS, "Usage: sv_addbot <name> <team>\n");
		return;
	}

	// TODO [Fifty]: Check teams to not script error

	const char* szName = args.Arg(1);
	int nTeam = std::atoi(args.Arg(2));

	g_pEngineServer->LockNetworkStringTables(true);

	edict_t nHandle = g_pEngineServer->CreateFakeClient(szName, "", "", nTeam);
	g_pServerGameClients->ClientFullyConnect(nHandle, false);

	g_pEngineServer->LockNetworkStringTables(false);
}
