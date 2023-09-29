#pragma once

#include "mathlib/bitbuf.h"
#include "tier1/utlmemory.h"

#include "engine/client/client.h"
#include "engine/baseserver.h"
#include "networksystem/inetmsghandler.h"

//
class CServer : public IConnectionlessPacketHandler
{
  public:
	// clang-format off
	int GetNumHumanPlayers(void) const;
	int GetNumFakeClients(void) const;
	int GetNumClients(void) const;

	inline bool IsActive() const { return m_State == server_state_t::ss_active; }
	inline bool IsLoading() const { return m_State == server_state_t::ss_loading; }
	inline bool IsPaused() const { return m_State == server_state_t::ss_paused; }
	inline bool IsDead() const { return m_State == server_state_t::ss_dead; }

	inline const char* GetMapName() const { return m_szMapName; }
	inline const char* GetMapGroupName() const { return m_szMapGroupName; }

	inline const char* GetPassword() const { return m_szPassword; }

	inline CClient* GetClient( int nIdx ) { return &m_Clients[nIdx]; } // FIXME [Fifty]: Add bounds checking
	// clang-format on

  public: // TODO: make private
	server_state_t m_State;
	int m_Socket;
	int m_nTickCount;
	bool m_bResetMaxTeams;
	char m_szMapName[64];
	char m_szMapGroupName[64];
	char m_szPassword[32];
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
	bf_write m_Signon;
	CUtlMemory<int8_t> m_SignonBuffer;
	int m_nServerClasses;
	int m_nServerClassBits;
	char m_szConDetails[64];
	char m_szHostInfo[128];
	char pad[46];
	CClient m_Clients[32];

	// TODO [Fifty]: Finish this class
};

inline CServer* g_pServer;

inline void (*CServer__RejectConnection)(CServer* self, int iSocket, void* unk, const char* fmt, ...);
