#include "game/server/util_server.h"
#include "engine/server/server.h"
#include "engine/client/client.h"
#include "engine/edict.h"

//-----------------------------------------------------------------------------
// Purpose: Runs null commands on fake players
//-----------------------------------------------------------------------------
void Physics_RunBotThinkFunctions(bool bSimulate)
{
	// NOTE [Fifty]: running null commands on bots fixes bots being stuck in the air
	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* pClient = g_pServer->GetClient(i);
		if (!pClient)
			continue;

		if (pClient->m_nSignonState == eSignonState::FULL && pClient->m_bFakePlayer)
		{
			CPlayer* pPlayer = UTIL_PlayerByIndex(pClient->m_nHandle);
			if (pPlayer)
			{
				CPlayer__RunNullCommand(pPlayer);
			}
		}
	}
}

void (*o_Physics_RunThinkFunctions)(bool bSimulate);

void h_Physics_RunThinkFunctions(bool bSimulate)
{
	Physics_RunBotThinkFunctions(bSimulate);
	o_Physics_RunThinkFunctions(bSimulate);
}

ON_DLL_LOAD("server.dll", PhysicsMain, (CModule module))
{
	o_Physics_RunThinkFunctions = module.Offset(0x483A50).RCast<void (*)(bool)>();
	HookAttach(&(PVOID&)o_Physics_RunThinkFunctions, (PVOID)h_Physics_RunThinkFunctions);
}
