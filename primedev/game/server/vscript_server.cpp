#include "squirrel/squirrelautobind.h"
#include "squirrel/squirrelclasstypes.h"
#include "squirrel/squirreldatatypes.h"
#include "squirrel/squirrel.h"

#include "game/server/player.h"
#include "engine/server/server.h"
#include "engine/client/client.h"
#include "game/server/gameinterface.h"
#include "originsdk/origin.h"

ADD_SQFUNC("void", NSEarlyWritePlayerPersistenceForLeave, "entity player", "", ScriptContext::SERVER)
{
	const CPlayer* pPlayer = g_pSquirrel<context>->template getentity<CPlayer>(sqvm, 1);
	
	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSIsWritingPlayerPersistence, "", "", ScriptContext::SERVER)
{
	g_pSquirrel<context>->pushbool(sqvm, false /*g_pMasterServerManager->m_bSavingPersistentData*/);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsPlayerLocalPlayer, "entity player", "", ScriptContext::SERVER)
{
	const CPlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->template getentity<CPlayer>(sqvm, 1);
	if (!pPlayer)
	{
		Warning(eLog::NS, "NSIsPlayerLocalPlayer got null player\n");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	CClient* pClient = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1);
	g_pSquirrel<context>->pushbool(sqvm, !strcmp(g_pLocalPlayerUserID, pClient->m_UID));
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsDedicated, "", "", ScriptContext::SERVER)
{
	g_pSquirrel<context>->pushbool(sqvm, IsDedicatedServer());
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSDisconnectPlayer, "entity player, string reason", "Disconnects the player from the server with the given reason", ScriptContext::SERVER)
{
	const CPlayer* pPlayer = g_pSquirrel<context>->template getentity<CPlayer>(sqvm, 1);
	const char* reason = g_pSquirrel<context>->getstring(sqvm, 2);

	if (!pPlayer)
	{
		Warning(eLog::NS, "Attempted to call NSDisconnectPlayer() with null player.\n");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	// Shouldn't happen but I like sanity checks.
	CClient* pClient = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1);
	if (!pClient)
	{
		Warning(eLog::NS, "NSDisconnectPlayer(): player entity has null CClient!\n");

		g_pSquirrel<context>->pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	if (reason)
	{
		CClient__Disconnect(pClient, 1, reason);
	}
	else
	{
		CClient__Disconnect(pClient, 1, "Disconnected by the server.");
	}

	g_pSquirrel<context>->pushbool(sqvm, true);
	return SQRESULT_NOTNULL;
}

// clang-format off
ADD_SQFUNC("string", GetUserInfoKVString_Internal, "entity player, string key, string defaultValue = \"\"",
	"Gets the string value of a given player's userinfo convar by name", ScriptContext::SERVER)
// clang-format on
{
	const CPlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->template getentity<CPlayer>(sqvm, 1);
	if (!pPlayer)
	{
		g_pSquirrel<ScriptContext::SERVER>->raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 2);
	const char* pDefaultValue = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 3);

	const char* pResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetString(pKey, pDefaultValue);
	g_pSquirrel<ScriptContext::SERVER>->pushstring(sqvm, pResult);
	return SQRESULT_NOTNULL;
}

// clang-format off
ADD_SQFUNC("asset", GetUserInfoKVAsset_Internal, "entity player, string key, asset defaultValue = $\"\"",
	"Gets the asset value of a given player's userinfo convar by name", ScriptContext::SERVER)
// clang-format on
{
	const CPlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->template getentity<CPlayer>(sqvm, 1);
	if (!pPlayer)
	{
		g_pSquirrel<ScriptContext::SERVER>->raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 2);
	const char* pDefaultValue;
	g_pSquirrel<ScriptContext::SERVER>->getasset(sqvm, 3, &pDefaultValue);

	const char* pResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetString(pKey, pDefaultValue);
	g_pSquirrel<ScriptContext::SERVER>->pushasset(sqvm, pResult);
	return SQRESULT_NOTNULL;
}

// clang-format off
ADD_SQFUNC("int", GetUserInfoKVInt_Internal, "entity player, string key, int defaultValue = 0",
	"Gets the int value of a given player's userinfo convar by name", ScriptContext::SERVER)
// clang-format on
{
	const CPlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->template getentity<CPlayer>(sqvm, 1);
	if (!pPlayer)
	{
		g_pSquirrel<ScriptContext::SERVER>->raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 2);
	const int iDefaultValue = g_pSquirrel<ScriptContext::SERVER>->getinteger(sqvm, 3);

	const int iResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetInt(pKey, iDefaultValue);
	g_pSquirrel<ScriptContext::SERVER>->pushinteger(sqvm, iResult);
	return SQRESULT_NOTNULL;
}

// clang-format off
ADD_SQFUNC("float", GetUserInfoKVFloat_Internal, "entity player, string key, float defaultValue = 0",
	"Gets the float value of a given player's userinfo convar by name", ScriptContext::SERVER)
// clang-format on
{
	const CPlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->getentity<CPlayer>(sqvm, 1);
	if (!pPlayer)
	{
		g_pSquirrel<ScriptContext::SERVER>->raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 2);
	const float flDefaultValue = g_pSquirrel<ScriptContext::SERVER>->getfloat(sqvm, 3);

	const float flResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetFloat(pKey, flDefaultValue);
	g_pSquirrel<ScriptContext::SERVER>->pushfloat(sqvm, flResult);
	return SQRESULT_NOTNULL;
}

// clang-format off
ADD_SQFUNC("bool", GetUserInfoKVBool_Internal, "entity player, string key, bool defaultValue = false",
	"Gets the bool value of a given player's userinfo convar by name", ScriptContext::SERVER)
// clang-format on
{
	const CPlayer* pPlayer = g_pSquirrel<ScriptContext::SERVER>->getentity<CPlayer>(sqvm, 1);
	if (!pPlayer)
	{
		g_pSquirrel<ScriptContext::SERVER>->raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 2);
	const bool bDefaultValue = g_pSquirrel<ScriptContext::SERVER>->getbool(sqvm, 3);

	const bool bResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetInt(pKey, bDefaultValue);
	g_pSquirrel<ScriptContext::SERVER>->pushbool(sqvm, bResult);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSSendMessage, "int playerIndex, string text, bool isTeam", "", ScriptContext::SERVER)
{
	int playerIndex = g_pSquirrel<ScriptContext::SERVER>->getinteger(sqvm, 1);
	const char* text = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 2);
	bool isTeam = g_pSquirrel<ScriptContext::SERVER>->getbool(sqvm, 3);

	Chat_SendMessage(playerIndex, text, isTeam);

	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSBroadcastMessage, "int fromPlayerIndex, int toPlayerIndex, string text, bool isTeam, bool isDead, int messageType", "", ScriptContext::SERVER)
{
	int fromPlayerIndex = g_pSquirrel<ScriptContext::SERVER>->getinteger(sqvm, 1);
	int toPlayerIndex = g_pSquirrel<ScriptContext::SERVER>->getinteger(sqvm, 2);
	const char* text = g_pSquirrel<ScriptContext::SERVER>->getstring(sqvm, 3);
	bool isTeam = g_pSquirrel<ScriptContext::SERVER>->getbool(sqvm, 4);
	bool isDead = g_pSquirrel<ScriptContext::SERVER>->getbool(sqvm, 5);
	int messageType = g_pSquirrel<ScriptContext::SERVER>->getinteger(sqvm, 6);

	if (messageType < 1)
	{
		g_pSquirrel<ScriptContext::SERVER>->raiseerror(sqvm, fmt::format("Invalid message type {}", messageType).c_str());
		return SQRESULT_ERROR;
	}

	Chat_BroadcastMessage(fromPlayerIndex, toPlayerIndex, text, isTeam, isDead, (CustomMessageType)messageType);

	return SQRESULT_NULL;
}
