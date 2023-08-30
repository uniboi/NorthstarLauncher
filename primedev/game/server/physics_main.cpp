AUTOHOOK_INIT()

#include "server/r2server.h"
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
			CBasePlayer* pPlayer = UTIL_PlayerByIndex(pClient->m_nHandle);
			if (pPlayer)
			{
				CBasePlayer__RunNullCommand(pPlayer);
			}
		}
	}
}

AUTOHOOK(Physics_RunThinkFunctions, server.dll + 0x483A50, void, __fastcall, (bool bSimulate))
{
	Physics_RunBotThinkFunctions(bSimulate);
	Physics_RunThinkFunctions(bSimulate);
}

ON_DLL_LOAD("server.dll", PhysicsMain, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
