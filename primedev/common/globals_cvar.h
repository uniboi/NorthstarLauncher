#pragma once

#include "tier1/convar.h"

//-----------------------------------------------------------------------------
// Dll callback
void CVar_InitModule(std::string svModule);

//-----------------------------------------------------------------------------
//
void CVar_InitShipped(std::string svModule);

//-----------------------------------------------------------------------------
// client.dll
extern ConVar* Cvar_ns_print_played_sounds;
extern ConVar* Cvar_r_latencyflex;
extern ConVar* Cvar_cl_showtextmsg;

extern ConVar* Cvar_demo_enableDemos;
extern ConVar* Cvar_demo_writeLocalFile;
extern ConVar* Cvar_demo_autoRecord;
extern ConVar* Cvar_mp_gamemode;

//-----------------------------------------------------------------------------
// server.dll
extern ConVar* Cvar_ns_ai_dumpAINfileFromLoad;
extern ConVar* Cvar_ns_exploitfixes_log;
extern ConVar* Cvar_ns_should_log_all_clientcommands;
extern ConVar* CVar_bot_pilot_settings;
extern ConVar* CVar_bot_force_pilot_primary;
extern ConVar* CVar_bot_force_pilot_secondary;
extern ConVar* CVar_bot_force_pilot_weapon3;
extern ConVar* CVar_bot_force_pilot_ordnance;
extern ConVar* CVar_bot_force_pilot_ability;
extern ConVar* CVar_bot_titan_settings;
extern ConVar* CVar_bot_force_titan_ordnance;
extern ConVar* Cvar_bot_force_titan_ability;

extern ConVar* Cvar_atlas_broadcast_local_server;

extern ConVar* Cvar_sv_cheats;
extern ConVar* Cvar_fatal_script_errors;
extern ConVar* Cvar_sv_alltalk;

//-----------------------------------------------------------------------------
// engine.dll
extern ConVar* Cvar_ns_has_agreed_to_send_token;
extern ConVar* Cvar_rui_drawEnable;
extern ConVar* Cvar_spewlog_enable;
extern ConVar* Cvar_ns_masterserver_hostname;
extern ConVar* Cvar_ns_curl_log_enable;
extern ConVar* Cvar_ns_prefer_datatable_from_disk;
extern ConVar* Cvar_ns_erase_auth_info;
extern ConVar* Cvar_ns_auth_allow_insecure;
extern ConVar* Cvar_ns_auth_allow_insecure_write;
extern ConVar* Cvar_net_debug_atlas_packet;
extern ConVar* Cvar_net_debug_atlas_packet_insecure;
extern ConVar* Cvar_ns_server_name;
extern ConVar* Cvar_ns_server_presence_update_rate;
extern ConVar* Cvar_ns_server_desc;
extern ConVar* Cvar_ns_server_password;
extern ConVar* Cvar_ns_report_server_to_masterserver;
extern ConVar* Cvar_ns_report_sp_server_to_masterserver;
extern ConVar* CVar_sv_quota_stringcmdspersecond;
extern ConVar* Cvar_net_chan_limit_mode;
extern ConVar* Cvar_net_chan_limit_msec_per_sec;
extern ConVar* Cvar_sv_querylimit_per_sec;
extern ConVar* Cvar_sv_max_chat_messages_per_sec;
extern ConVar* Cvar_sv_antispeedhack_enable;
extern ConVar* Cvar_sv_antispeedhack_maxtickbudget;
extern ConVar* Cvar_sv_antispeedhack_budgetincreasemultiplier;
extern ConVar* Cvar_ns_use_clc_SetPlaylistVarOverride;
extern ConVar* Cvar_hostdescription;
extern ConVar* Cvar_hostpassword;
extern ConVar* Cvar_navmesh_debug_hull;
extern ConVar* Cvar_navmesh_debug_camera_radius;
extern ConVar* Cvar_navmesh_debug_draw_type;

extern ConVar* Cvar_atlas_hostname;

extern ConVar* Cvar_hostname;
extern ConVar* Cvar_hostport;

extern ConVar* Cvar_enable_debug_overlays;
extern ConVar* Cvar_dedi_sendPrintsToClient;

extern ConVar* Cvar_ai_script_nodes_draw;
