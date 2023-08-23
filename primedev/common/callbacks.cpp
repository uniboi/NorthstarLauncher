#include "common/callbacks.h"

#include "server/serverpresence.h"

#include "engine/cdll_engine_int.h"
#include "client/localchatwriter.h"
#include "gameui/GameConsole.h"
#include "mods/modmanager.h"
#include "networksystem/masterserver.h"
#include "squirrel/squirrel.h"
#include "scripts/scriptdatatables.h"
#include "server/auth/bansystem.h"
#include "engine/r2engine.h"
#include "engine/client/client.h"
#include "engine/server/server.h"
#include "server/auth/serverauthentication.h"
#include "engine/hoststate.h"
#include "client/r2client.h"
#include "shared/playlist.h"
#include "util/printcommands.h"

//-----------------------------------------------------------------------------
// Purpose: 
//-----------------------------------------------------------------------------
void NS_ServerName_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	g_pServerPresence->SetName(UnescapeUnicode(Cvar_ns_server_name->GetString()));

	// update engine hostname cvar
	Cvar_hostname->SetValue(Cvar_ns_server_name->GetString());
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerDesc_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	g_pServerPresence->SetDescription(UnescapeUnicode(Cvar_ns_server_desc->GetString()));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void NS_ServerPass_f(ConVar* cvar, const char* pOldValue, float flOldValue)
{
	g_pServerPresence->SetPassword(Cvar_ns_server_password->GetString());
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
void CC_ToggleConsole_f(const CCommand& args) {
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
	g_pMasterServerManager->RequestServerList();
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
		CClient* player = &g_pServer->m_Clients[i];

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
void CC_ns_resetpersistence_f(const CCommand& args)
{
	if (g_pServer->m_State == server_state_t::ss_active)
	{
		Error(eLog::NS, NO_ERROR, "ns_resetpersistence must be entered from the main menu\n");
		return;
	}

	Warning(eLog::NS, "resetting persistence on next lobby load...\n");
	g_pServerAuthentication->m_bForceResetLocalPlayerPersistence = true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ns_start_reauth_and_leave_to_lobby_f(const CCommand& arg)
{
	// hack for special case where we're on a local server, so we erase our own newly created auth data on disconnect
	g_pMasterServerManager->m_bNewgameAfterSelfAuth = true;
	g_pMasterServerManager->AuthenticateWithOwnServer(g_pLocalPlayerUserID, g_pMasterServerManager->m_sOwnClientAuthToken);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_ns_end_reauth_and_leave_to_lobby_f(const CCommand& arg)
{
	if (g_pServerAuthentication->m_RemoteAuthenticationData.size())
		g_pCVar->FindVar("serverfilter")->SetValue(g_pServerAuthentication->m_RemoteAuthenticationData.begin()->first.c_str());

	// weird way of checking, but check if client script vm is initialised, mainly just to allow players to cancel this
	if (g_pSquirrel<ScriptContext::CLIENT>->m_pSQVM)
	{
		g_pServerAuthentication->m_bNeedLocalAuthForNewgame = true;

		// this won't set playlist correctly on remote clients, don't think they can set playlist until they've left which sorta
		// fucks things should maybe set this in HostState_NewGame?
		R2::SetCurrentPlaylist("tdm");
		strcpy(g_pHostState->m_levelName, "mp_lobby");
		g_pHostState->m_iNextState = HostState_t::HS_NEW_GAME;
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_playlist_f(const CCommand& args)
{
	if (args.ArgC() < 2)
		return;

	R2::SetCurrentPlaylist(args.Arg(1));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_setplaylistvaroverride_f(const CCommand& args)
{
	if (args.ArgC() < 3)
		return;

	for (int i = 1; i < args.ArgC(); i += 2)
		R2::SetPlaylistVarOverride(args.Arg(i), args.Arg(i + 1));
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

	char pTempName[256];
	char pTempSearchTerm[256];

	for (auto& map : g_pCVar->DumpToMap())
	{
		bool bPrintCommand = true;
		for (int i = 0; i < arg.ArgC() - 1; i++)
		{
			// make lowercase to avoid case sensitivity
			strncpy_s(pTempName, sizeof(pTempName), map.second->m_pszName, sizeof(pTempName) - 1);
			strncpy_s(pTempSearchTerm, sizeof(pTempSearchTerm), arg.Arg(i + 1), sizeof(pTempSearchTerm) - 1);

			for (int i = 0; pTempName[i]; i++)
				pTempName[i] = tolower(pTempName[i]);

			for (int i = 0; pTempSearchTerm[i]; i++)
				pTempSearchTerm[i] = tolower(pTempSearchTerm[i]);

			if (!strstr(pTempName, pTempSearchTerm))
			{
				bPrintCommand = false;
				break;
			}
		}

		if (bPrintCommand)
			PrintCommandHelpDialogue(map.second, map.second->m_pszName);
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CC_findflags_f(const CCommand& arg)
{
	if (arg.ArgC() < 2)
	{
		DevMsg(eLog::NS, "Usage: findflags <string>\n");
		for (auto& flagPair : g_PrintCommandFlags)
			DevMsg(eLog::NS, "   - %i\n", flagPair.second);

		return;
	}

	// convert input flag to uppercase
	char* upperFlag = new char[strlen(arg.Arg(1))];
	strcpy(upperFlag, arg.Arg(1));

	for (int i = 0; upperFlag[i]; i++)
		upperFlag[i] = toupper(upperFlag[i]);

	// resolve flag name => int flags
	int resolvedFlag = FCVAR_NONE;
	for (auto& flagPair : g_PrintCommandFlags)
	{
		if (!strcmp(flagPair.second, upperFlag))
		{
			resolvedFlag |= flagPair.first;
			break;
		}
	}

	// print cvars
	for (auto& map : g_pCVar->DumpToMap())
	{
		if (map.second->m_nFlags & resolvedFlag)
			PrintCommandHelpDialogue(map.second, map.second->m_pszName);
	}

	delete[] upperFlag;
}
