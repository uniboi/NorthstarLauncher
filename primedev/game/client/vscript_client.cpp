#include "game/client/vscript_client.h"

#include "vscript/vscript.h"
#include "mathlib/vector.h"
#include "client/localchatwriter.h"
#include "windows/window.h"
#include "originsdk/origin.h"
#include "networksystem/atlas.h"
#include "engine/cl_splitscreen.h"
#include "engine/edict.h"
#include "engine/server/server.h"

SQRESULT Script_NSChatWrite(HSQUIRRELVM sqvm)
{
	int chatContext = sq_getinteger(sqvm, 1);
	const char* str = sq_getstring(sqvm, 2);

	LocalChatWriter((LocalChatWriter::Context)chatContext).Write(str);
	return SQRESULT_NULL;
}
SQRESULT Script_NSChatWriteRaw(HSQUIRRELVM sqvm)
{
	int chatContext = sq_getinteger(sqvm, 1);
	const char* str = sq_getstring(sqvm, 2);

	LocalChatWriter((LocalChatWriter::Context)chatContext).InsertText(str);
	return SQRESULT_NULL;
}
SQRESULT Script_NSChatWriteLine(HSQUIRRELVM sqvm)
{
	int chatContext = sq_getinteger(sqvm, 1);
	const char* str = sq_getstring(sqvm, 2);

	LocalChatWriter((LocalChatWriter::Context)chatContext).WriteLine(str);
	return SQRESULT_NULL;
}

SQRESULT Script_NSGetCursorPosition(HSQUIRRELVM sqvm)
{
	RECT rcClient;
	POINT p;
	if (GetCursorPos(&p) && ScreenToClient(*g_hGameWindow, &p) && GetClientRect(*g_hGameWindow, &rcClient))
	{
		if (GetAncestor(GetForegroundWindow(), GA_ROOTOWNER) != *g_hGameWindow)
			return SQRESULT_NULL;

		Vector3 vPos = Vector3(p.x > 0 ? p.x > rcClient.right ? rcClient.right : (float)p.x : 0, p.y > 0 ? p.y > rcClient.bottom ? rcClient.bottom : (float)p.y : 0, 0);

		sq_pushvector(sqvm, (SQFloat*)&vPos);
		return SQRESULT_NOTNULL;
	}
	sq_raiseerror(sqvm, "Failed retrieving cursor position of game window");
	return SQRESULT_ERROR;
}

enum eMainMenuPromoDataProperty
{
	newInfoTitle1,
	newInfoTitle2,
	newInfoTitle3,

	largeButtonTitle,
	largeButtonText,
	largeButtonUrl,
	largeButtonImageIndex,

	smallButton1Title,
	smallButton1Url,
	smallButton1ImageIndex,

	smallButton2Title,
	smallButton2Url,
	smallButton2ImageIndex
};

SQRESULT Script_NSRequestCustomMainMenuPromos(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	// g_pMasterServerManager->RequestMainMenuPromos();
	return SQRESULT_NULL;
}

SQRESULT Script_NSHasCustomMainMenuPromoData(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, true /*g_pMasterServerManager->m_bHasMainMenuPromoData*/);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSGetCustomMainMenuPromoData(HSQUIRRELVM sqvm)
{
	int nPromoDataKey = sq_getinteger(sqvm, 1);
	NOTE_UNUSED(nPromoDataKey);
	switch (sq_getinteger(sqvm, 1))
	{
	case eMainMenuPromoDataProperty::newInfoTitle1:
	case eMainMenuPromoDataProperty::newInfoTitle2:
	case eMainMenuPromoDataProperty::newInfoTitle3:
	case eMainMenuPromoDataProperty::largeButtonTitle:
	case eMainMenuPromoDataProperty::largeButtonText:
	case eMainMenuPromoDataProperty::largeButtonUrl:
	case eMainMenuPromoDataProperty::smallButton1Title:
	case eMainMenuPromoDataProperty::smallButton1Url:
	case eMainMenuPromoDataProperty::smallButton2Title:
	case eMainMenuPromoDataProperty::smallButton2Url:
	{
		sq_pushstring(sqvm, "STUB", -1);
		break;
	}
	case eMainMenuPromoDataProperty::smallButton1ImageIndex:
	{
		sq_pushinteger(sqvm, 0);
		break;
	}
	case eMainMenuPromoDataProperty::smallButton2ImageIndex:
	{
		sq_pushinteger(sqvm, 1);
		break;
	}

	case eMainMenuPromoDataProperty::largeButtonImageIndex:
	{
		sq_pushinteger(sqvm, 2);
		break;
	}
	}

	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSIsMasterServerAuthenticated(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, g_pAtlasClient->GetOriginAuthSuccessful());
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSRequestServerList(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	g_pAtlasClient->FetchRemoteGameServerList();
	return SQRESULT_NULL;
}

SQRESULT Script_NSIsRequestingServerList(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, g_pAtlasClient->GetFetchingRemoteGameServers());
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSMasterServerConnectionSuccessful(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, true /*g_pMasterServerManager->m_bSuccessfullyConnected*/);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSGetServerCount(HSQUIRRELVM sqvm)
{
	sq_pushinteger(sqvm, g_pAtlasClient->GetRemoteGameServerList().size());
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSClearRecievedServerList(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	g_pAtlasClient->ClearRemoteGameServerList();
	return SQRESULT_NULL;
}

// functions for authenticating with servers

SQRESULT Script_NSTryAuthWithServer(HSQUIRRELVM sqvm)
{
	SQInteger serverIndex = sq_getinteger(sqvm, 1);
	const SQChar* password = sq_getstring(sqvm, 2);

	if (strncmp(g_pServerGlobalVariables->m_pMapName, "mp_lobby", 9))
	{
		sq_raiseerror(sqvm, "Can only run NSTryAuthWithServer while in mp_lobby");
		return SQRESULT_ERROR;
	}

	int nNumServers = g_pAtlasClient->GetRemoteGameServerList().size();

	if (serverIndex >= nNumServers || serverIndex < 0)
	{
		sq_raiseerror(sqvm, FormatA("Tried to auth with server index %i when only %i servers are available", serverIndex, nNumServers).c_str());
		return SQRESULT_ERROR;
	}

	for (int i = 0; i < g_pServer->GetNumClients(); i++)
	{
		CClient* pClient = g_pServer->GetClient(i);

		if (pClient->m_bFakePlayer)
			continue;

		g_pAtlasServer->PushPersistence(pClient);
	}

	g_pAtlasClient->AuthenticateRemoteGameServer(g_pLocalPlayerUserID, password, g_pAtlasClient->GetRemoteGameServerList().at(serverIndex));

	return SQRESULT_NULL;
}

SQRESULT Script_NSIsAuthenticatingWithServer(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, false /*g_pMasterServerManager->m_bScriptAuthenticatingWithGameServer*/);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSWasAuthSuccessful(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, true /*g_pMasterServerManager->m_bSuccessfullyAuthenticatedWithGameServer*/);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSConnectToAuthedServer(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	return SQRESULT_NULL;
}

SQRESULT Script_NSTryAuthWithLocalServer(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	return SQRESULT_NULL;
}

SQRESULT Script_NSCompleteAuthWithLocalServer(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	return SQRESULT_NULL;
}

SQRESULT Script_NSGetAuthFailReason(HSQUIRRELVM sqvm)
{
	sq_pushstring(sqvm, "STUB", -1);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSGetGameServers(HSQUIRRELVM sqvm)
{
	sq_newarray(sqvm, 0);
	size_t nIdx = 0;
	for (const RemoteGameServer_t& remoteServer : g_pAtlasClient->GetRemoteGameServerList())
	{
		sq_pushnewstructinstance(sqvm, 11);

		// index
		sq_pushinteger(sqvm, nIdx++);
		sq_sealstructslot(sqvm, 0);

		// id
		sq_pushstring(sqvm, remoteServer.m_svID.c_str(), -1);
		sq_sealstructslot(sqvm, 1);

		// name
		sq_pushstring(sqvm, remoteServer.m_svName.c_str(), -1);
		sq_sealstructslot(sqvm, 2);

		// description
		sq_pushstring(sqvm, remoteServer.m_svDescription.c_str(), -1);
		sq_sealstructslot(sqvm, 3);

		// map
		sq_pushstring(sqvm, remoteServer.m_svMap.c_str(), -1);
		sq_sealstructslot(sqvm, 4);

		// playlist
		sq_pushstring(sqvm, remoteServer.m_svPlaylist.c_str(), -1);
		sq_sealstructslot(sqvm, 5);

		// playerCount
		sq_pushinteger(sqvm, remoteServer.m_nPlayerCount);
		sq_sealstructslot(sqvm, 6);

		// maxPlayerCount
		sq_pushinteger(sqvm, remoteServer.m_nMaxPlayers);
		sq_sealstructslot(sqvm, 7);

		// requiresPassword
		sq_pushbool(sqvm, remoteServer.m_bRequiresPassword);
		sq_sealstructslot(sqvm, 8);

		// region
		sq_pushstring(sqvm, remoteServer.m_svRegion.c_str(), -1);
		sq_sealstructslot(sqvm, 9);

		// requiredMods
		sq_newarray(sqvm, 0);
		for (const RemoteGameMod_t& mod : remoteServer.m_vRequiredMods)
		{
			sq_pushnewstructinstance(sqvm, 2);

			// name
			sq_pushstring(sqvm, mod.m_svName.c_str(), -1);
			sq_sealstructslot(sqvm, 0);

			// version
			sq_pushstring(sqvm, mod.m_svVersion.c_str(), -1);
			sq_sealstructslot(sqvm, 1);

			sq_arrayappend(sqvm, -2);
		}
		sq_sealstructslot(sqvm, 10);

		sq_arrayappend(sqvm, -2);
	}
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSPushGameStateData(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	// SQStructInstance* structInst = g_pSquirrel<ScriptContext::CLIENT>->m_pSQVM->GetVM()->_stackOfCurrentFunction[1]._VAL.asStructInstance;

	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSPushUIPresence(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	// SQStructInstance* structInst = g_pSquirrel<ScriptContext::UI>->m_pSQVM->GetVM()->_stackOfCurrentFunction[1]._VAL.asStructInstance;

	return SQRESULT_NOTNULL;
}

void VScript_RegisterClientFunctions(CSquirrelVM* vm)
{
	vm->RegisterFunction("NSChatWrite", "Script_NSChatWrite", "", "void", "int context, string text", Script_NSChatWrite);
	vm->RegisterFunction("NSChatWriteRaw", "Script_NSChatWriteRaw", "", "void", "int context, string text", Script_NSChatWriteRaw);
	vm->RegisterFunction("NSChatWriteLine", "Script_NSChatWriteLine", "", "void", "int context, string text", Script_NSChatWriteLine);
	vm->RegisterFunction("NSPushGameStateData", "Script_NSPushGameStateData", "", "void", "GameStateStruct gamestate", Script_NSPushGameStateData);
}

void VScript_RegisterUIFunctions(CSquirrelVM* vm)
{
	vm->RegisterFunction("NSGetCursorPosition", "Script_NSGetCursorPosition", "", "vector ornull", "", Script_NSGetCursorPosition);
	vm->RegisterFunction("NSRequestCustomMainMenuPromos", "Script_NSRequestCustomMainMenuPromos", "", "void", "", Script_NSRequestCustomMainMenuPromos);
	vm->RegisterFunction("NSHasCustomMainMenuPromoData", "Script_NSHasCustomMainMenuPromoData", "", "bool", "", Script_NSHasCustomMainMenuPromoData);
	vm->RegisterFunction("NSGetCustomMainMenuPromoData", "Script_NSGetCustomMainMenuPromoData", "", "var", "int promoDataKey", Script_NSGetCustomMainMenuPromoData);
	vm->RegisterFunction("NSIsMasterServerAuthenticated", "Script_NSIsMasterServerAuthenticated", "", "bool", "", Script_NSIsMasterServerAuthenticated);
	vm->RegisterFunction("NSRequestServerList", "Script_NSRequestServerList", "", "void", "", Script_NSRequestServerList);
	vm->RegisterFunction("NSIsRequestingServerList", "Script_NSIsRequestingServerList", "", "bool", "", Script_NSIsRequestingServerList);
	vm->RegisterFunction("NSMasterServerConnectionSuccessful", "Script_NSMasterServerConnectionSuccessful", "", "bool", "", Script_NSMasterServerConnectionSuccessful);
	vm->RegisterFunction("NSGetServerCount", "Script_NSGetServerCount", "", "int", "", Script_NSGetServerCount);
	vm->RegisterFunction("NSClearRecievedServerList", "Script_NSClearRecievedServerList", "", "void", "", Script_NSClearRecievedServerList);
	vm->RegisterFunction("NSTryAuthWithServer", "Script_NSTryAuthWithServer", "", "void", "int serverIndex, string password = ''", Script_NSTryAuthWithServer);
	vm->RegisterFunction("NSIsAuthenticatingWithServer", "Script_NSIsAuthenticatingWithServer", "", "bool", "", Script_NSIsAuthenticatingWithServer);
	vm->RegisterFunction("NSWasAuthSuccessful", "Script_NSWasAuthSuccessful", "", "bool", "", Script_NSWasAuthSuccessful);
	vm->RegisterFunction("NSConnectToAuthedServer", "Script_NSConnectToAuthedServer", "", "void", "", Script_NSConnectToAuthedServer);
	vm->RegisterFunction("NSTryAuthWithLocalServer", "Script_NSTryAuthWithLocalServer", "", "void", "", Script_NSTryAuthWithLocalServer);
	vm->RegisterFunction("NSCompleteAuthWithLocalServer", "Script_NSCompleteAuthWithLocalServer", "", "void", "", Script_NSCompleteAuthWithLocalServer);
	vm->RegisterFunction("NSGetAuthFailReason", "Script_NSGetAuthFailReason", "", "string", "", Script_NSGetAuthFailReason);
	vm->RegisterFunction("NSGetGameServers", "Script_NSGetGameServers", "", "array<ServerInfo>", "", Script_NSGetGameServers);
	vm->RegisterFunction("NSPushUIPresence", "Script_NSPushUIPresence", "", "void", "UIPresenceStruct presence", Script_NSPushUIPresence);
}
