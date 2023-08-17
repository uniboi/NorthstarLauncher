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
		//ns_test = ConVar::StaticCreate("ns_test", "0", FCVAR_NONE, "Testing");

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
		Cvar_ns_server_name = ConVar::StaticCreate("ns_server_name", "Unnamed Northstar Server", FCVAR_GAMEDLL, "This server's description", false, 0.0, false, 0.0, &Host_ServerName_f);
		Cvar_ns_server_presence_update_rate = ConVar::StaticCreate("ns_server_presence_update_rate", "5000", FCVAR_GAMEDLL, "How often we update our server's presence on server lists in ms");
		Cvar_ns_server_desc = ConVar::StaticCreate("ns_server_desc", "Default server description", FCVAR_GAMEDLL, "This server's name", false, 0, false, 0, &Host_ServerDesc_f);
		Cvar_ns_server_password = ConVar::StaticCreate("ns_server_password", "", FCVAR_GAMEDLL, "This server's password", false, 0, false, 0, &Host_ServerPass_f);
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

		DevMsg(eLog::NS, "Finished initilasing new cvars for '%s'\n", "engine.dll");
		return;
	}

	//Warning(eLog::NS, "Tried to initilase new cvars for '%s'\n", svModule.c_str());
}

//-----------------------------------------------------------------------------
// Purpose: Initilases retail cvars
//          Needs to be called after ConVar_Register is called in the module!!!
// Input  : svModule - The module we initilase cvars for
//-----------------------------------------------------------------------------
void CVar_InitShipped(std::string svModule)
{
	// client.dll
	static bool bClient = false;
	if (svModule == "client.dll" && !bClient)
	{
		Cvar_demo_enableDemos = g_pCVar->FindVar("demo_enabledemos");
		Cvar_demo_writeLocalFile = g_pCVar->FindVar("demo_writeLocalFile");
		Cvar_demo_autoRecord = g_pCVar->FindVar("demo_autoRecord");
		Cvar_mp_gamemode = g_pCVar->FindVar("mp_gamemode");

		// demo_enableDemos
		Cvar_demo_enableDemos->m_pszDefaultValue = "1";
		Cvar_demo_enableDemos->SetValue(true);

		// demo_writeLocalFile
		Cvar_demo_writeLocalFile->m_pszDefaultValue = "1";
		Cvar_demo_writeLocalFile->SetValue(true);

		// demo_autoRecord
		Cvar_demo_autoRecord->m_pszDefaultValue = "0";
		Cvar_demo_autoRecord->SetValue(false);

		DevMsg(eLog::NS, "Finished initilasing shipped cvars for '%s'\n", "client.dll");
		return;
	}

	// server.dll
	static bool bServer = false;
	if (svModule == "server.dll" && !bServer)
	{
		bServer = true;

		Cvar_sv_cheats = g_pCVar->FindVar("sv_cheats");
		Cvar_fatal_script_errors = g_pCVar->FindVar("fatal_script_errors");
		Cvar_sv_alltalk = g_pCVar->FindVar("sv_alltalk");

		DevMsg(eLog::NS, "Finished initilasing shipped cvars for '%s'\n", "server.dll");
		return;
	}

	// engine.dll
	static bool bEngine = false;
	if (svModule == "engine.dll" && !bEngine)
	{
		bEngine = true;

		Cvar_hostname = g_pCVar->FindVar("hostname");
		Cvar_enable_debug_overlays = g_pCVar->FindVar("enable_debug_overlays");
		Cvar_dedi_sendPrintsToClient = g_pCVar->FindVar("dedi_sendPrintsToClient");
		Cvar_hostport = g_pCVar->FindVar("hostport");

		// enable_debug_overlays
		Cvar_enable_debug_overlays->SetValue(false);
		Cvar_enable_debug_overlays->m_pszDefaultValue = (char*)"0";
		Cvar_enable_debug_overlays->AddFlags(FCVAR_CHEAT);

		DevMsg(eLog::NS, "Finished initilasing shipped cvars for '%s'\n", "engine.dll");
		return;
	}

	// Warning(eLog::NS, "Tried to initilase shipped cvars for '%s'\n", svModule.c_str());
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
