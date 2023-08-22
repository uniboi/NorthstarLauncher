#include "common/callbacks.h"

#include "server/serverpresence.h"

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
