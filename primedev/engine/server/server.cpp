#include "engine/server/server.h"

#include "engine/client/client.h"
#include "engine/edict.h"
#include "shared/exploit_fixes/ns_limits.h"
#include "networksystem/atlas.h"
#include "networksystem/bansystem.h"

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

CClient* (*o_CServer__ConnectClient)(CServer* self, void* a2, __int64 a3, unsigned int a4, unsigned int a5, int a6, __int64 a7, const char* pszPlayerName, const char* pszServerFilter, __int64 a10, char a11, void* a12, char a13, char a14, uint64_t nUID,
									 unsigned int a16, unsigned int a17);

CClient* h_CServer__ConnectClient(CServer* self, void* a2, __int64 a3, unsigned int a4, unsigned int a5, int a6, __int64 a7, const char* pszPlayerName, const char* pszServerFilter, __int64 a10, char a11, void* a12, char a13, char a14, uint64_t nUID,
								  unsigned int a16, unsigned int a17)
{
	// We use serverfilter to correlate client with auth data ( like pdata )
	if (!g_pAtlasServer->HasAuthInfo(pszServerFilter))
	{
		CServer__RejectConnection(self, self->m_Socket, a2, "An error most likely occured when authenticating server-side!\n\nCode: INVALID_SERVERFILTER");
		return nullptr;
	}

	if (!g_pBanSystem->IsUIDAllowed(nUID))
	{
		CServer__RejectConnection(self, self->m_Socket, a2, "Banned from server\n\nCode: PLAYER_BANNED");
		return nullptr;
	}

	// Verify player name
	AuthInfo_t info = g_pAtlasServer->GetAuthInfo(pszServerFilter);
	std::string svPlayerName = pszPlayerName;
	if (!info.m_svName.empty())
	{
		svPlayerName = info.m_svName;
	}

	// Make sure it's only ASCII characters
	for (int i = 0; i < svPlayerName.size(); i++)
	{
		if (svPlayerName[i] < 32 || svPlayerName[i] > 126)
		{
			CServer__RejectConnection(self, self->m_Socket, a2, "Playername contains illegal characters!\n\nCode: INVALID_PLAYERNAME");
			return nullptr;
		}
	}

	// Connect the client
	CClient* pClient = o_CServer__ConnectClient(self, a2, a3, a4, a5, a6, a7, svPlayerName.c_str(), pszServerFilter, a10, a11, a12, a13, a14, nUID, a16, a17);

	if (!pClient)
		return nullptr;

	// Set uid
	std::string svUID = std::to_string(nUID);
	strncpy(pClient->m_UID, svUID.c_str(), 32);

	// Setup atlas info ( pdata )
	if (!g_pAtlasServer->SetupClient(pClient, pszServerFilter))
	{
		CClient__Disconnect(pClient, 1, "Failed to setup client!");
	}

	// No need to keep this after connecting the client
	g_pAtlasServer->RemoveAuthInfo(pszServerFilter);

	g_pServerLimits->AddPlayer(pClient);

	return pClient;
}

bool (*o_CServer__ProcessConnectionlessPacket)(CServer* self, netpacket_t* packet);

bool h_CServer__ProcessConnectionlessPacket(CServer* self, netpacket_t* packet)
{
	// packet->data consists of 0xFFFFFFFF (int32 -1) to indicate packets aren't split, followed by a header consisting of a single
	// character, which is used to uniquely identify the packet kind. Most kinds follow this with a null-terminated string payload
	// then an arbitrary amount of data.

	// T (no rate limits since we authenticate packets before doing anything expensive)
	if (4 < packet->size && packet->data[4] == 'T')
	{
		g_pAtlasServer->HandleConnectionlessPacket(packet);
		return false;
	}

	// check rate limits for the original unconnected packets
	if (!g_pServerLimits->CheckConnectionlessPacketLimits(packet))
		return false;

	// A, H, I, N
	return o_CServer__ProcessConnectionlessPacket(self, packet);
}

ON_DLL_LOAD("engine.dll", EngineServer, (CModule module))
{
	o_CServer__ConnectClient = module.Offset(0x114430).RCast<CClient* (*)(CServer*, void*, __int64, unsigned int, unsigned int, int, __int64, const char*, const char*, __int64, char, void*, char, char, uint64_t, unsigned int, unsigned int)>();
	HookAttach(&(PVOID&)o_CServer__ConnectClient, (PVOID)h_CServer__ConnectClient);

	o_CServer__ProcessConnectionlessPacket = module.Offset(0x117800).RCast<bool (*)(CServer*, netpacket_t*)>();
	HookAttach(&(PVOID&)o_CServer__ProcessConnectionlessPacket, (PVOID)h_CServer__ProcessConnectionlessPacket);

	g_pServer = module.Offset(0x12A53D40).RCast<CServer*>();

	CServer__RejectConnection = module.Offset(0x1182E0).RCast<void (*)(CServer*, int, void*, const char*, ...)>();
}
