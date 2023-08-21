#pragma once

#include "shared/keyvalues.h"

inline void (*CClient__Disconnect)(void* self, uint32_t unknownButAlways1, const char* reason, ...);

// #56169 $DB69 PData size
// #512   $200	Trailing data
// #100	  $64	Safety buffer
const int PERSISTENCE_MAX_SIZE = 0xDDCD;

// note: NOT_READY and READY are the only entries we have here that are defined by the vanilla game
// entries after this are custom and used to determine the source of persistence, e.g. whether it is local or remote
enum class ePersistenceReady : char
{
	NOT_READY,
	READY = 3,
	READY_INSECURE = 3,
	READY_REMOTE
};

enum class eSignonState : int
{
	NONE = 0, // no state yet; about to connect
	CHALLENGE = 1, // client challenging server; all OOB packets
	CONNECTED = 2, // client is connected to server; netchans ready
	NEW = 3, // just got serverinfo and string tables
	PRESPAWN = 4, // received signon buffers
	GETTINGDATA = 5, // respawn-defined signonstate, assumedly this is for persistence
	SPAWN = 6, // ready to receive entity packets
	FIRSTSNAP = 7, // another respawn-defined one
	FULL = 8, // we are fully connected; first non-delta packet received
	CHANGELEVEL = 9, // server is changing level; please wait
};

//-----------------------------------------------------------------------------
//
class CClient
{
	void* vftable;
	void* vftable2;

  public:
	uint32_t m_nUserID;
	uint16_t m_nHandle;
	char m_szServerName[64];
	int64_t m_nReputation;
	char pad_0014[182];
	char m_szClientName[64];
	char pad_0015[252];
	KeyValues* m_ConVars;
	char pad_0368[8];
	void* m_pServer;
	char pad_0378[32];
	void* m_NetChannel;
	char pad_03A8[8];
	eSignonState m_nSignonState;
	int32_t m_nDeltaTick;
	uint64_t m_nOriginID;
	int32_t m_nStringTableAckTick;
	int32_t m_nSignonTick;
	char pad_03C0[160];
	char m_szClanTag[16];
	char pad2[284];
	bool m_bFakePlayer;
	bool m_bReceivedPacket;
	bool m_bLowViolence;
	bool m_bFullyAuthenticated;
	char pad_05A4[24];
	ePersistenceReady m_nPersistenceState;
	char pad_05C0[89];
	char m_PersistenceBuffer[PERSISTENCE_MAX_SIZE];
	char pad[4665];
	char m_UID[32];
	char pad0[0x1E208];
};
static_assert(sizeof(CClient) == 0x2D728);

// [Fifty]: Sanity checks
static_assert(offsetof(CClient, m_szServerName) == 0x16);
static_assert(offsetof(CClient, m_ConVars) == 0x258);
static_assert(offsetof(CClient, m_nSignonState) == 0x2A0);
static_assert(offsetof(CClient, m_szClanTag) == 0x358);
static_assert(offsetof(CClient, m_bFakePlayer) == 0x484);
static_assert(offsetof(CClient, m_nPersistenceState) == 0x4A0);
static_assert(offsetof(CClient, m_PersistenceBuffer) == 0x4FA);
static_assert(offsetof(CClient, m_UID) == 0xF500);
