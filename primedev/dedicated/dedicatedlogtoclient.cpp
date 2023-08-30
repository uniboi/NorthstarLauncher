#include "dedicatedlogtoclient.h"

#include "tier1/convar.h"
#include "engine/server/server.h"
#include "engine/edict.h"
#include "logging/logging.h"

void (*CGameClient__ClientPrintf)(CClient* pClient, const char* fmt, ...);

void DediClientMsg(const char* pszMessage)
{
	if (g_pServer == NULL || g_pCVar == NULL)
		return;

	if (g_pServer->IsDead())
		return;

	enum class eSendPrintsToClient
	{
		NONE = -1,
		FIRST,
		ALL
	};

	eSendPrintsToClient eSendPrints = static_cast<eSendPrintsToClient>(Cvar_dedi_sendPrintsToClient->GetInt());
	if (eSendPrints == eSendPrintsToClient::NONE)
		return;

	std::string sLogMessage = fmt::format("{}", pszMessage);
	for (int i = 0; i < g_pServerGlobalVariables->m_nMaxClients; i++)
	{
		CClient* pClient = g_pServer->GetClient(i);

		if (pClient->m_nSignonState >= eSignonState::CONNECTED)
		{
			CGameClient__ClientPrintf(pClient, sLogMessage.c_str());

			if (eSendPrints == eSendPrintsToClient::FIRST)
				break;
		}
	}
}

ON_DLL_LOAD_DEDI("engine.dll", DedicatedServerLogToClient, (CModule module))
{
	CGameClient__ClientPrintf = module.Offset(0x1016A0).RCast<void (*)(CClient*, const char*, ...)>();
}
