#pragma once

#include "tier1/cmd.h"
#include "shared/keyvalues.h"
#include "engine/edict.h"

extern void (*CBaseClient__Disconnect)(void* self, uint32_t unknownButAlways1, const char* reason, ...);

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

// clang-format off
OFFSET_STRUCT(CBaseClient)
{
	STRUCT_SIZE(0x2D728)
	FIELD(0x16, char m_Name[64])
	FIELD(0x258, KeyValues* m_ConVars)
	FIELD(0x2A0, eSignonState m_Signon)
	FIELD(0x358, char m_ClanTag[16])
	FIELD(0x484, bool m_bFakePlayer)
	FIELD(0x4A0, ePersistenceReady m_iPersistenceReady)
	FIELD(0x4FA, char m_PersistenceBuffer[PERSISTENCE_MAX_SIZE])
	FIELD(0xF500, char m_UID[32])
};
// clang-format on

extern CBaseClient* g_pClientArray;

enum server_state_t
{
	ss_dead = 0, // Dead
	ss_loading, // Spawning
	ss_active, // Running
	ss_paused, // Running, but paused
};

extern server_state_t* g_pServerState;

extern char* g_pModName;

extern CGlobalVars* g_pServerGlobalVariables;
extern CGlobalVarsBase* g_pClientGlobalVariables;
