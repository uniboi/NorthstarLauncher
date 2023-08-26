#pragma once

void NS_ServerName_f(ConVar* cvar, const char* pOldValue, float flOldValue);
void NS_ServerDesc_f(ConVar* cvar, const char* pOldValue, float flOldValue);
void NS_ServerPass_f(ConVar* cvar, const char* pOldValue, float flOldValue);

void CC_dump_datamap(const CCommand& args);

void CC_Say_f(const CCommand& args);
void CC_SayTeam_f(const CCommand& args);
void CC_Log_f(const CCommand& args);

void CC_ToggleConsole_f(const CCommand& args);
void CC_ShowConsole_f(const CCommand& args);
void CC_HideConsole_f(const CCommand& args);

void CC_reload_mods_f(const CCommand& args);

void CC_ns_fetchservers_f(const CCommand& args);

void CC_ns_script_servertoclientstringcommand_f(const CCommand& arg);

void CC_dump_datatable_f(const CCommand& args);
void CC_dump_datatables_f(const CCommand& args);

void CC_ban_f(const CCommand& args);
void CC_unban_f(const CCommand& args);
void CC_clearbanlist_f(const CCommand& args);

void CC_ns_resetpersistence_f(const CCommand& args);

void CC_ns_start_reauth_and_leave_to_lobby_f(const CCommand& arg);
void CC_ns_end_reauth_and_leave_to_lobby_f(const CCommand& arg);

void CC_playlist_f(const CCommand& args);
void CC_setplaylistvaroverride_f(const CCommand& args);

void CC_script_ui_f(const CCommand& args);
void CC_script_cl_f(const CCommand& args);
void CC_script_sv_f(const CCommand& args);

void CC_find_f(const CCommand& arg);
void CC_help_f(const CCommand& arg);
void CC_printcvaraddr_f(const CCommand& arg);
