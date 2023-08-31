#include "gamepresence.h"
#include "dedicated/dedicated.h"
#include "server/serverpresence.h"
#include "networksystem/masterserver.h"
#include "squirrel/squirrel.h"

GameStatePresence* g_pGameStatePresence;

GameStatePresence::GameStatePresence()
{
	g_pServerPresence->AddPresenceReporter(&m_GameStateServerPresenceReporter);
}

void GameStateServerPresenceReporter::RunFrame(double flCurrentTime, const ServerPresence* pServerPresence)
{
	g_pGameStatePresence->id = pServerPresence->m_sServerId;
	g_pGameStatePresence->name = pServerPresence->m_sServerName;
	g_pGameStatePresence->description = pServerPresence->m_sServerDesc;
	g_pGameStatePresence->password = pServerPresence->m_Password;

	g_pGameStatePresence->map = pServerPresence->m_MapName;
	g_pGameStatePresence->playlist = pServerPresence->m_PlaylistName;
	g_pGameStatePresence->currentPlayers = pServerPresence->m_iPlayerCount;
	g_pGameStatePresence->maxPlayers = pServerPresence->m_iMaxPlayers;

	g_pGameStatePresence->isLocal = !IsDedicatedServer();
}

void GameStatePresence::RunFrame()
{
	if (g_pSquirrel<ScriptContext::UI>->m_pSQVM != nullptr && g_pSquirrel<ScriptContext::UI>->m_pSQVM->sqvm != nullptr)
		g_pSquirrel<ScriptContext::UI>->Call("NorthstarCodeCallback_GenerateUIPresence");

	if (g_pSquirrel<ScriptContext::CLIENT>->m_pSQVM != nullptr && g_pSquirrel<ScriptContext::CLIENT>->m_pSQVM->sqvm != nullptr)
	{
		auto test = g_pSquirrel<ScriptContext::CLIENT>->Call("NorthstarCodeCallback_GenerateGameState");
	}
}
