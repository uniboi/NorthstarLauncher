#include "game/server/vscript_server.h"

#include "vscript/vscript.h"

#include "game/server/player.h"
#include "engine/server/server.h"
#include "engine/client/client.h"
#include "game/server/gameinterface.h"
#include "originsdk/origin.h"

SQRESULT Script_NSEarlyWritePlayerPersistenceForLeave(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	NOTE_UNUSED(pPlayer);
	return SQRESULT_NULL;
}

SQRESULT Script_NSIsWritingPlayerPersistence(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, false /*g_pMasterServerManager->m_bSavingPersistentData*/);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSIsPlayerLocalPlayer(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	if (!pPlayer)
	{
		Warning(eLog::NS, "NSIsPlayerLocalPlayer got null player\n");

		sq_pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	CClient* pClient = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1);
	sq_pushbool(sqvm, !strcmp(g_pLocalPlayerUserID, pClient->m_UID));
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSIsDedicated(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, IsDedicatedServer());
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSDisconnectPlayer(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	const char* reason = sq_getstring(sqvm, 2);

	if (!pPlayer)
	{
		Warning(eLog::NS, "Attempted to call NSDisconnectPlayer() with null player.\n");

		sq_pushbool(sqvm, false);
		return SQRESULT_NOTNULL;
	}

	// Shouldn't happen but I like sanity checks.
	CClient* pClient = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1);
	if (!pClient)
	{
		Warning(eLog::NS, "NSDisconnectPlayer(): player entity has null CClient!\n");

		sq_pushbool(sqvm, false);
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

	sq_pushbool(sqvm, true);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_GetUserInfoKVString_Internal(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	if (!pPlayer)
	{
		sq_raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = sq_getstring(sqvm, 2);
	const char* pDefaultValue = sq_getstring(sqvm, 3);

	const char* pResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetString(pKey, pDefaultValue);
	sq_pushstring(sqvm, pResult, -1);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_GetUserInfoKVAsset_Internal(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	if (!pPlayer)
	{
		sq_raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = sq_getstring(sqvm, 2);
	const char* pDefaultValue;
	sq_getasset(sqvm, 3, &pDefaultValue);

	const char* pResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetString(pKey, pDefaultValue);
	sq_pushasset(sqvm, pResult, -1);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_GetUserInfoKVInt_Internal(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	if (!pPlayer)
	{
		sq_raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = sq_getstring(sqvm, 2);
	const int iDefaultValue = sq_getinteger(sqvm, 3);

	const int iResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetInt(pKey, iDefaultValue);
	sq_pushinteger(sqvm, iResult);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_GetUserInfoKVFloat_Internal(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	if (!pPlayer)
	{
		sq_raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = sq_getstring(sqvm, 2);
	const float flDefaultValue = sq_getfloat(sqvm, 3);

	const float flResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetFloat(pKey, flDefaultValue);
	sq_pushfloat(sqvm, flResult);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_GetUserInfoKVBool_Internal(HSQUIRRELVM sqvm)
{
	// FIXME [Fifty]: Verify we're given CPlayer*
	const CPlayer* pPlayer = (CPlayer*)sq_getentity(sqvm, 1);
	if (!pPlayer)
	{
		sq_raiseerror(sqvm, "player is null");
		return SQRESULT_ERROR;
	}

	const char* pKey = sq_getstring(sqvm, 2);
	const bool bDefaultValue = sq_getbool(sqvm, 3);

	const bool bResult = g_pServer->GetClient(pPlayer->m_Network.m_edict - 1)->m_ConVars->GetInt(pKey, bDefaultValue);
	sq_pushbool(sqvm, bResult);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSSendMessage(HSQUIRRELVM sqvm)
{
	int playerIndex = sq_getinteger(sqvm, 1);
	const char* text = sq_getstring(sqvm, 2);
	bool isTeam = sq_getbool(sqvm, 3);

	Chat_SendMessage(playerIndex, text, isTeam);

	return SQRESULT_NULL;
}

SQRESULT Script_NSBroadcastMessage(HSQUIRRELVM sqvm)
{
	int fromPlayerIndex = sq_getinteger(sqvm, 1);
	int toPlayerIndex = sq_getinteger(sqvm, 2);
	const char* text = sq_getstring(sqvm, 3);
	bool isTeam = sq_getbool(sqvm, 4);
	bool isDead = sq_getbool(sqvm, 5);
	int messageType = sq_getinteger(sqvm, 6);

	if (messageType < 1)
	{
		sq_raiseerror(sqvm, FormatA("Invalid message type %i", messageType).c_str());
		return SQRESULT_ERROR;
	}

	Chat_BroadcastMessage(fromPlayerIndex, toPlayerIndex, text, isTeam, isDead, (CustomMessageType)messageType);

	return SQRESULT_NULL;
}

void VScript_RegisterServerFunctions(CSquirrelVM* vm)
{
	vm->RegisterFunction("NSEarlyWritePlayerPersistenceForLeave", "Script_NSEarlyWritePlayerPersistenceForLeave", "", "void", "entity player", Script_NSEarlyWritePlayerPersistenceForLeave);
	vm->RegisterFunction("NSIsWritingPlayerPersistence", "Script_NSIsWritingPlayerPersistence", "", "bool", "", Script_NSIsWritingPlayerPersistence);
	vm->RegisterFunction("NSIsPlayerLocalPlayer", "Script_NSIsPlayerLocalPlayer", "", "bool", "entity player", Script_NSIsPlayerLocalPlayer);
	vm->RegisterFunction("NSIsDedicated", "Script_NSIsDedicated", "", "bool", "", Script_NSIsDedicated);
	vm->RegisterFunction("NSDisconnectPlayer", "Script_NSDisconnectPlayer", "", "bool", "entity player, string reason", Script_NSDisconnectPlayer);
	vm->RegisterFunction("GetUserInfoKVString_Internal", "Script_GetUserInfoKVString_Internal", "", "string", "entity player, string key, string defaultValue = \"\"", Script_GetUserInfoKVString_Internal);
	vm->RegisterFunction("GetUserInfoKVAsset_Internal", "Script_GetUserInfoKVAsset_Internal", "", "asset", "entity player, string key, asset defaultValue = $\"\"", Script_GetUserInfoKVAsset_Internal);
	vm->RegisterFunction("GetUserInfoKVInt_Internal", "Script_GetUserInfoKVInt_Internal", "", "int", "entity player, string key, int defaultValue = 0", Script_GetUserInfoKVInt_Internal);
	vm->RegisterFunction("GetUserInfoKVFloat_Internal", "Script_GetUserInfoKVFloat_Internal", "", "float", "entity player, string key, float defaultValue = 0", Script_GetUserInfoKVFloat_Internal);
	vm->RegisterFunction("GetUserInfoKVBool_Internal", "Script_GetUserInfoKVBool_Internal", "", "bool", "entity player, string key, bool defaultValue = false", Script_GetUserInfoKVBool_Internal);
	vm->RegisterFunction("NSSendMessage", "Script_NSSendMessage", "", "void", "int playerIndex, string text, bool isTeam", Script_NSSendMessage);
	vm->RegisterFunction("NSBroadcastMessage", "Script_NSBroadcastMessage", "", "void", "int fromPlayerIndex, int toPlayerIndex, string text, bool isTeam, bool isDead, int messageType", Script_NSBroadcastMessage);
}
