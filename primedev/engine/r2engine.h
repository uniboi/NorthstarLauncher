#pragma once

#include "tier1/cmd.h"
#include "shared/keyvalues.h"
#include "engine/edict.h"
#include "engine/client/client.h"

extern CClient* g_pClientArray;

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
