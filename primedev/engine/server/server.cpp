#include "engine/server/server.h"

#include "engine/edict.h"

//-----------------------------------------------------------------------------
// Purpose: Returns number of connected human players
//-----------------------------------------------------------------------------
int CServer::GetNumHumanPlayers(void) const
{
	int nClients = 0;
	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* pClient = g_pServer->GetClient(i);

		if (!pClient)
			continue;

		if (pClient->m_nSignonState == eSignonState::FULL && !pClient->m_bFakePlayer)
			nClients++;
	}

	return nClients;
}

//-----------------------------------------------------------------------------
// Purpose: Returns number of connected fake players
//-----------------------------------------------------------------------------
int CServer::GetNumFakeClients(void) const
{
	int nClients = 0;
	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* pClient = g_pServer->GetClient(i);

		if (!pClient)
			continue;

		if (pClient->m_nSignonState == eSignonState::FULL && pClient->m_bFakePlayer)
			nClients++;
	}

	return nClients;
}

//-----------------------------------------------------------------------------
// Purpose: Returns number of connected clients
//-----------------------------------------------------------------------------
int CServer::GetNumClients(void) const
{
	int nClients = 0;
	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* pClient = g_pServer->GetClient(i);

		if (!pClient)
			continue;

		if (pClient->m_nSignonState == eSignonState::FULL)
			nClients++;
	}

	return nClients;
}


ON_DLL_LOAD("engine.dll", EngineServer, (CModule module))
{
	g_pServer = module.Offset(0x12A53D40).RCast<CServer*>();
}
