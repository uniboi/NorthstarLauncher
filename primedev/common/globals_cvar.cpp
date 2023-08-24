#include "common/globals_cvar.h"

#include "common/callbacks.h"

std::vector<std::string> vInitilasedModules;

//-----------------------------------------------------------------------------
// Purpose: Initilases our new cvars
//          Gets called right after the detour callbacks
// Input  : svModule - The module for for which we're creating cvars
// Note   : Due to the game being dynamically linked we need to init cvars
//          in the dll load callback for the modules where they may be used
//-----------------------------------------------------------------------------
void CVar_InitModule(std::string svModule)
{
	// client.dll
	static bool bClient = false;
	if (svModule == "client.dll" && !bClient)
	{
		bClient = true;

		Cvar_ns_print_played_sounds = ConVar::StaticCreate("ns_print_played_sounds", "0", FCVAR_NONE, "");
		Cvar_r_latencyflex = ConVar::StaticCreate("r_latencyflex", "1", FCVAR_ARCHIVE, "Whether or not to use LatencyFleX input latency reduction.");
		Cvar_cl_showtextmsg = ConVar::StaticCreate("cl_showtextmsg", "1", FCVAR_NONE, "Enable/disable text messages printing on the screen.");
		// ns_test = ConVar::StaticCreate("ns_test", "0", FCVAR_NONE, "Testing");

		ConCommand::StaticCreate("script_client", "Executes script code on the client vm", FCVAR_CLIENTDLL, CC_script_cl_f, nullptr);
		ConCommand::StaticCreate("script_ui", "Executes script code on the ui vm", FCVAR_CLIENTDLL, CC_script_ui_f, nullptr);

		if (!IsDedicatedServer()) // CLIENT
		{
			ConCommand::StaticCreate("toggleconsole", "Show/hide the console.", FCVAR_DONTRECORD, CC_ToggleConsole_f, nullptr);
			ConCommand::StaticCreate("showconsole", "Show the console.", FCVAR_DONTRECORD, CC_ShowConsole_f, nullptr);
			ConCommand::StaticCreate("hideconsole", "Hide the console.", FCVAR_DONTRECORD, CC_HideConsole_f, nullptr);
		
			ConCommand::StaticCreate("ns_script_servertoclientstringcommand", "", FCVAR_CLIENTDLL | FCVAR_SERVER_CAN_EXECUTE, CC_ns_script_servertoclientstringcommand_f, nullptr);
		}

		DevMsg(eLog::NS, "Finished initilasing new cvars for '%s'\n", "client.dll");
		return;
	}

	// server.dll
	static bool bServer = false;
	if (svModule == "server.dll" && !bServer)
	{
		bServer = true;

		Cvar_ns_ai_dumpAINfileFromLoad = ConVar::StaticCreate("ns_ai_dumpAINfileFromLoad", "0", FCVAR_NONE, "For debugging: whether we should dump ain data for ains loaded from disk");
		Cvar_ns_exploitfixes_log = ConVar::StaticCreate("ns_exploitfixes_log", "1", FCVAR_GAMEDLL, "Whether to log whenever ExploitFixes.cpp blocks/corrects something");
		Cvar_ns_should_log_all_clientcommands = ConVar::StaticCreate("ns_should_log_all_clientcommands", "0", FCVAR_NONE, "Whether to log all clientcommands");

		ConCommand::StaticCreate("dump_datamap", "", FCVAR_NONE, CC_dump_datamap, nullptr);

		// FCVAR_CHEAT and FCVAR_GAMEDLL_FOR_REMOTE_CLIENTS allows clients to execute this, but since it's unsafe we only allow it when cheats
		// are enabled for script_client and script_ui, we don't use cheats, so clients can execute them on themselves all they want
		ConCommand::StaticCreate("script", "Executes script code on the server vm", FCVAR_GAMEDLL | FCVAR_GAMEDLL_FOR_REMOTE_CLIENTS | FCVAR_CHEAT, CC_script_sv_f, nullptr);

		DevMsg(eLog::NS, "Finished initilasing new cvars for '%s'\n", "server.dll");
		return;
	}

	// engine.dll
	static bool bEngine = false;
	if (svModule == "engine.dll" && !bEngine)
	{
		bEngine = true;

		Cvar_ns_has_agreed_to_send_token = ConVar::StaticCreate("ns_has_agreed_to_send_token", "1", FCVAR_ARCHIVE_PLAYERPROFILE, "whether the user has agreed to send their origin token to the northstar masterserver");
		Cvar_rui_drawEnable = ConVar::StaticCreate("rui_drawEnable", "1", FCVAR_CLIENTDLL, "Controls whether RUI should be drawn");
		Cvar_spewlog_enable = ConVar::StaticCreate("spewlog_enable", "1", FCVAR_NONE, "Enables/disables whether the engine spewfunc should be logged");
		Cvar_ns_masterserver_hostname = ConVar::StaticCreate("ns_masterserver_hostname", "127.0.0.1", FCVAR_NONE, "");
		Cvar_ns_curl_log_enable = ConVar::StaticCreate("ns_curl_log_enable", "0", FCVAR_NONE, "Whether curl should log to the console");
		Cvar_ns_prefer_datatable_from_disk = ConVar::StaticCreate("ns_prefer_datatable_from_disk", IsDedicatedServer() && CommandLine()->CheckParm("-nopakdedi") ? "1" : "0", FCVAR_NONE, "whether to prefer loading datatables from disk, rather than rpak");
		Cvar_ns_erase_auth_info = ConVar::StaticCreate("ns_erase_auth_info", "1", FCVAR_GAMEDLL, "Whether auth info should be erased from this server on disconnect or crash");
		Cvar_ns_auth_allow_insecure = ConVar::StaticCreate("ns_auth_allow_insecure", "0", FCVAR_GAMEDLL, "Whether this server will allow unauthenicated players to connect");
		Cvar_ns_auth_allow_insecure_write = ConVar::StaticCreate("ns_auth_allow_insecure_write", "0", FCVAR_GAMEDLL, "Whether the pdata of unauthenticated clients will be written to disk when changed");
		Cvar_net_debug_atlas_packet = ConVar::StaticCreate("net_debug_atlas_packet", "0", FCVAR_NONE, "Whether to log detailed debugging information for Atlas connectionless packets (warning: this allows unlimited amounts of arbitrary data to be logged)");
		Cvar_net_debug_atlas_packet_insecure = ConVar::StaticCreate("net_debug_atlas_packet_insecure", "0", FCVAR_NONE, "Whether to disable signature verification for Atlas connectionless packets (DANGEROUS: this allows anyone to impersonate Atlas)");
		Cvar_ns_server_name = ConVar::StaticCreate("ns_server_name", "Unnamed Northstar Server", FCVAR_GAMEDLL, "This server's description", false, 0.0, false, 0.0, &NS_ServerName_f);
		Cvar_ns_server_presence_update_rate = ConVar::StaticCreate("ns_server_presence_update_rate", "5000", FCVAR_GAMEDLL, "How often we update our server's presence on server lists in ms");
		Cvar_ns_server_desc = ConVar::StaticCreate("ns_server_desc", "Default server description", FCVAR_GAMEDLL, "This server's name", false, 0, false, 0, &NS_ServerDesc_f);
		Cvar_ns_server_password = ConVar::StaticCreate("ns_server_password", "", FCVAR_GAMEDLL, "This server's password", false, 0, false, 0, &NS_ServerPass_f);
		Cvar_ns_report_server_to_masterserver = ConVar::StaticCreate("ns_report_server_to_masterserver", "1", FCVAR_GAMEDLL, "Whether we should report this server to the masterserver");
		Cvar_ns_report_sp_server_to_masterserver = ConVar::StaticCreate("ns_report_sp_server_to_masterserver", "0", FCVAR_GAMEDLL, "Whether we should report this server to the masterserver, when started in singleplayer");
		CVar_sv_quota_stringcmdspersecond = ConVar::StaticCreate("sv_quota_stringcmdspersecond", "60", FCVAR_GAMEDLL, "How many string commands per second clients are allowed to submit, 0 to disallow all string commands, -1 to disable");
		Cvar_net_chan_limit_mode = ConVar::StaticCreate("net_chan_limit_mode", "0", FCVAR_GAMEDLL, "The mode for netchan processing limits: 0 = warn, 1 = kick");
		Cvar_net_chan_limit_msec_per_sec = ConVar::StaticCreate("net_chan_limit_msec_per_sec", "100", FCVAR_GAMEDLL, "Netchannel processing is limited to so many milliseconds, abort connection if exceeding budget");
		Cvar_sv_querylimit_per_sec = ConVar::StaticCreate("sv_querylimit_per_sec", "15", FCVAR_GAMEDLL, "");
		Cvar_sv_max_chat_messages_per_sec = ConVar::StaticCreate("sv_max_chat_messages_per_sec", "5", FCVAR_GAMEDLL, "");
		Cvar_sv_antispeedhack_enable = ConVar::StaticCreate("sv_antispeedhack_enable", "0", FCVAR_NONE, "whether to enable antispeedhack protections");
		Cvar_sv_antispeedhack_maxtickbudget = ConVar::StaticCreate("sv_antispeedhack_maxtickbudget", "64", FCVAR_GAMEDLL, "Maximum number of client-issued usercmd ticks that can be replayed in packet loss conditions");
		Cvar_sv_antispeedhack_budgetincreasemultiplier = ConVar::StaticCreate("sv_antispeedhack_budgetincreasemultiplier", "1", FCVAR_GAMEDLL, "Increase usercmd processing budget by tickinterval * value per tick");

		// note: clc_SetPlaylistVarOverride is pretty insecure, since it allows for entirely arbitrary playlist var overrides to be sent to the
		// server, this is somewhat restricted on custom servers to prevent it being done outside of private matches, but ideally it should be
		// disabled altogether, since the custom menus won't use it anyway this should only really be accepted if you want vanilla client
		// compatibility
		Cvar_ns_use_clc_SetPlaylistVarOverride = ConVar::StaticCreate("ns_use_clc_SetPlaylistVarOverride", "0", FCVAR_GAMEDLL, "Whether the server should accept clc_SetPlaylistVarOverride messages");

		ConCommand::StaticCreate("reload_mods", "reloads mods", FCVAR_NONE, CC_reload_mods_f, nullptr);
		ConCommand::StaticCreate("ns_fetchservers", "Fetch all servers from the masterserver", FCVAR_CLIENTDLL, CC_ns_fetchservers_f, nullptr);

		ConCommand::StaticCreate("dump_datatables", "dumps all datatables from a hardcoded list", FCVAR_NONE, CC_dump_datatables_f, nullptr);
		ConCommand::StaticCreate("dump_datatable", "dump a datatable", FCVAR_NONE, CC_dump_datatable_f, nullptr);

		ConCommand::StaticCreate("ban", "bans a given player by uid or name", FCVAR_GAMEDLL, CC_ban_f, nullptr);
		ConCommand::StaticCreate("unban", "unbans a given player by uid", FCVAR_GAMEDLL, CC_unban_f, nullptr);
		ConCommand::StaticCreate("clearbanlist", "clears all uids on the banlist", FCVAR_GAMEDLL, CC_clearbanlist_f, nullptr);

		ConCommand::StaticCreate("ns_resetpersistence", "resets your pdata when you next enter the lobby", FCVAR_NONE, CC_ns_resetpersistence_f, nullptr);

		ConCommand::StaticCreate("ns_start_reauth_and_leave_to_lobby", "called by the server, used to reauth and return the player to lobby when leaving a game", FCVAR_SERVER_CAN_EXECUTE, CC_ns_start_reauth_and_leave_to_lobby_f, nullptr);
		ConCommand::StaticCreate("ns_end_reauth_and_leave_to_lobby", "", FCVAR_NONE, CC_ns_end_reauth_and_leave_to_lobby_f, nullptr);

		// playlist is the name of the command on respawn servers, but we already use setplaylist so can't get rid of it
		ConCommand::StaticCreate("playlist", "Sets the current playlist", FCVAR_NONE, CC_playlist_f, nullptr);
		ConCommand::StaticCreate("setplaylist", "Sets the current playlist", FCVAR_NONE, CC_playlist_f, nullptr);
		ConCommand::StaticCreate("setplaylistvaroverrides", "sets a playlist var override", FCVAR_NONE, CC_setplaylistvaroverride_f, nullptr);

		ConCommand::StaticCreate("find", "Find concommands with the specified string in their name/help text.", FCVAR_NONE, CC_find_f, nullptr);
		ConCommand::StaticCreate("findflags", "Find concommands by flags.", FCVAR_NONE, CC_findflags_f, nullptr);

		if (!IsDedicatedServer()) // CLIENT
		{
			ConCommand::StaticCreate("say", "Enters a message in public chat", FCVAR_CLIENTDLL, CC_Say_f, nullptr);
			ConCommand::StaticCreate("say_team", "Enters a message in team chat", FCVAR_CLIENTDLL, CC_SayTeam_f, nullptr);
			ConCommand::StaticCreate("log", "Log a message to the local chat window", FCVAR_CLIENTDLL, CC_Log_f, nullptr);
		}

		DevMsg(eLog::NS, "Finished initilasing new cvars for '%s'\n", "engine.dll");
		return;
	}

	// Warning(eLog::NS, "Tried to initilase new cvars for '%s'\n", svModule.c_str());
}

//-----------------------------------------------------------------------------
// Purpose: Initilases retail cvars
//          Needs to be called after ConVar_Register is called in the module!!!
//-----------------------------------------------------------------------------
void CVar_InitShipped(std::string svModule)
{
	// Some convars may be initilased in different modules based on whether we're dedi or not
	// due to this we init them the first time they're registered regardless of module

	// demo_enableDemos
	if (!Cvar_demo_enableDemos && (Cvar_demo_enableDemos = g_pCVar->FindVar("demo_enabledemos")))
	{
		Cvar_demo_enableDemos->m_pszDefaultValue = "1";
		Cvar_demo_enableDemos->SetValue(true);
	}

	// demo_writeLocalFile
	if (!Cvar_demo_writeLocalFile && (Cvar_demo_writeLocalFile = g_pCVar->FindVar("demo_writeLocalFile")))
	{
		Cvar_demo_writeLocalFile->m_pszDefaultValue = "1";
		Cvar_demo_writeLocalFile->SetValue(true);
	}

	// demo_autoRecord
	if (!Cvar_demo_autoRecord && (Cvar_demo_autoRecord = g_pCVar->FindVar("demo_autoRecord")))
	{
		Cvar_demo_autoRecord->m_pszDefaultValue = "0";
		Cvar_demo_autoRecord->SetValue(false);
	}

	// mp_gamemode
	if (!Cvar_mp_gamemode && (Cvar_mp_gamemode = g_pCVar->FindVar("mp_gamemode")))
	{
		//
	}

	// sv_cheats
	if (!Cvar_sv_cheats && (Cvar_sv_cheats = g_pCVar->FindVar("sv_cheats")))
	{
		//
	}

	// fatal_script_errors
	if (!Cvar_fatal_script_errors && (Cvar_fatal_script_errors = g_pCVar->FindVar("fatal_script_errors")))
	{
		//
	}

	// sv_alltalk
	if (!Cvar_sv_alltalk && (Cvar_sv_alltalk = g_pCVar->FindVar("sv_alltalk")))
	{
		//
	}

	// hostname
	if (!Cvar_hostname && (Cvar_hostname = g_pCVar->FindVar("hostname")))
	{
		//
	}

	// enable_debug_overlays
	if (!Cvar_enable_debug_overlays && (Cvar_enable_debug_overlays = g_pCVar->FindVar("enable_debug_overlays")))
	{
		Cvar_enable_debug_overlays->SetValue(false);
		Cvar_enable_debug_overlays->m_pszDefaultValue = (char*)"0";
		Cvar_enable_debug_overlays->AddFlags(FCVAR_CHEAT);
	}

	// dedi_sendPrintsToClient
	if (!Cvar_dedi_sendPrintsToClient && (Cvar_dedi_sendPrintsToClient = g_pCVar->FindVar("dedi_sendPrintsToClient")))
	{
		//
	}

	// hostport
	if (!Cvar_hostport && (Cvar_hostport = g_pCVar->FindVar("hostport")))
	{
		//
	}

	DevMsg(eLog::NS, "Finished initilasing shipped cvars for '%s'\n", svModule.c_str());
}

//-----------------------------------------------------------------------------
// client.dll
ConVar* Cvar_ns_print_played_sounds = nullptr;
ConVar* Cvar_r_latencyflex = nullptr;
ConVar* Cvar_cl_showtextmsg = nullptr;

ConVar* Cvar_demo_enableDemos = nullptr;
ConVar* Cvar_demo_writeLocalFile = nullptr;
ConVar* Cvar_demo_autoRecord = nullptr;
ConVar* Cvar_mp_gamemode = nullptr;

//-----------------------------------------------------------------------------
// server.dll
ConVar* Cvar_ns_ai_dumpAINfileFromLoad = nullptr;
ConVar* Cvar_ns_exploitfixes_log = nullptr;
ConVar* Cvar_ns_should_log_all_clientcommands = nullptr;

ConVar* Cvar_sv_cheats = nullptr;
ConVar* Cvar_fatal_script_errors = nullptr;
ConVar* Cvar_sv_alltalk = nullptr;

//-----------------------------------------------------------------------------
// engine.dll
ConVar* Cvar_ns_has_agreed_to_send_token = nullptr;
ConVar* Cvar_rui_drawEnable = nullptr;
ConVar* Cvar_spewlog_enable = nullptr;
ConVar* Cvar_ns_masterserver_hostname = nullptr;
ConVar* Cvar_ns_curl_log_enable = nullptr;
ConVar* Cvar_ns_prefer_datatable_from_disk = nullptr;
ConVar* Cvar_ns_erase_auth_info = nullptr;
ConVar* Cvar_ns_auth_allow_insecure = nullptr;
ConVar* Cvar_ns_auth_allow_insecure_write = nullptr;
ConVar* Cvar_net_debug_atlas_packet = nullptr;
ConVar* Cvar_net_debug_atlas_packet_insecure = nullptr;
ConVar* Cvar_ns_server_name = nullptr;
ConVar* Cvar_ns_server_presence_update_rate = nullptr;
ConVar* Cvar_ns_server_desc = nullptr;
ConVar* Cvar_ns_server_password = nullptr;
ConVar* Cvar_ns_report_server_to_masterserver = nullptr;
ConVar* Cvar_ns_report_sp_server_to_masterserver = nullptr;
ConVar* CVar_sv_quota_stringcmdspersecond = nullptr;
ConVar* Cvar_net_chan_limit_mode = nullptr;
ConVar* Cvar_net_chan_limit_msec_per_sec = nullptr;
ConVar* Cvar_sv_querylimit_per_sec = nullptr;
ConVar* Cvar_sv_max_chat_messages_per_sec = nullptr;
ConVar* Cvar_sv_antispeedhack_enable = nullptr;
ConVar* Cvar_sv_antispeedhack_maxtickbudget = nullptr;
ConVar* Cvar_sv_antispeedhack_budgetincreasemultiplier = nullptr;
ConVar* Cvar_ns_use_clc_SetPlaylistVarOverride = nullptr;

ConVar* Cvar_hostname = nullptr;
ConVar* Cvar_enable_debug_overlays = nullptr;
ConVar* Cvar_dedi_sendPrintsToClient = nullptr;
ConVar* Cvar_hostport = nullptr;
