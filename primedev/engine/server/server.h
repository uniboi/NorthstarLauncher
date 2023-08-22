#pragma once

#include "mathlib/bitbuf.h"
#include "tier1/utlmemory.h"

#include "engine/client/client.h"
#include "engine/baseserver.h"

//
class CServer
{
	void* vftable;// IConnectionlessPacketHandler
  public:
	server_state_t m_State;
	int m_Socket;
	int m_nTickCount;
	bool m_bResetMaxTeams;
	char m_szMapname[64];
	char m_szMapGroupName[64];
	char m_Password[32];
	uint32_t worldmapCRC;
	uint32_t clientDllCRC;
	void* unkData;
	void /*CNetworkStringTableContainer*/* m_StringTables;
	void /*CNetworkStringTable*/* m_pInstanceBaselineTable;
	void /*CNetworkStringTable*/* m_pLightStyleTable;
	void /*CNetworkStringTable*/* m_pUserInfoTable;
	void /*CNetworkStringTable*/* m_pServerQueryTable;
	bool m_bReplay;
	bool m_bUpdateFrame;
	bool m_bUseReputation;
	bool m_bSimulating;
	int m_nPad;
	BFWrite m_Signon;
	CUtlMemory<int8_t> m_SignonBuffer;
	int m_nServerClasses;
	int m_nServerClassBits;
	char m_szConDetails[64];
	char m_szHostInfo[128];
	char pad[46];
	CClient m_Clients[32];

	// TODO [Fifty]: Finish this class
};
static_assert(offsetof(CServer, m_Clients) == 0x250);

inline CServer* g_pServer;
