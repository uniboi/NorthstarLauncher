#include "squirrel/squirrelautobind.h"
#include "squirrel/squirrelclasstypes.h"
#include "squirrel/squirreldatatypes.h"
#include "squirrel/squirrel.h"

#include "client/localchatwriter.h"
#include "windows/window.h"
#include "originsdk/origin.h"
#include "networksystem/atlas.h"
#include "engine/cl_splitscreen.h"
#include "engine/edict.h"
#include "engine/server/server.h"

ADD_SQFUNC("void", NSChatWrite, "int context, string text", "", ScriptContext::CLIENT)
{
	int chatContext = g_pSquirrel<ScriptContext::CLIENT>->getinteger(sqvm, 1);
	const char* str = g_pSquirrel<ScriptContext::CLIENT>->getstring(sqvm, 2);

	LocalChatWriter((LocalChatWriter::Context)chatContext).Write(str);
	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSChatWriteRaw, "int context, string text", "", ScriptContext::CLIENT)
{
	int chatContext = g_pSquirrel<ScriptContext::CLIENT>->getinteger(sqvm, 1);
	const char* str = g_pSquirrel<ScriptContext::CLIENT>->getstring(sqvm, 2);

	LocalChatWriter((LocalChatWriter::Context)chatContext).InsertText(str);
	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSChatWriteLine, "int context, string text", "", ScriptContext::CLIENT)
{
	int chatContext = g_pSquirrel<ScriptContext::CLIENT>->getinteger(sqvm, 1);
	const char* str = g_pSquirrel<ScriptContext::CLIENT>->getstring(sqvm, 2);

	LocalChatWriter((LocalChatWriter::Context)chatContext).WriteLine(str);
	return SQRESULT_NULL;
}

ADD_SQFUNC("vector ornull", NSGetCursorPosition, "", "", ScriptContext::UI)
{
	RECT rcClient;
	POINT p;
	if (GetCursorPos(&p) && ScreenToClient(*g_hGameWindow, &p) && GetClientRect(*g_hGameWindow, &rcClient))
	{
		if (GetAncestor(GetForegroundWindow(), GA_ROOTOWNER) != *g_hGameWindow)
			return SQRESULT_NULL;

		g_pSquirrel<context>->pushvector(sqvm, {p.x > 0 ? p.x > rcClient.right ? rcClient.right : (float)p.x : 0, p.y > 0 ? p.y > rcClient.bottom ? rcClient.bottom : (float)p.y : 0, 0});
		return SQRESULT_NOTNULL;
	}
	g_pSquirrel<context>->raiseerror(sqvm, "Failed retrieving cursor position of game window");
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

ADD_SQFUNC("void", NSRequestCustomMainMenuPromos, "", "", ScriptContext::UI)
{
	// g_pMasterServerManager->RequestMainMenuPromos();
	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSHasCustomMainMenuPromoData, "", "", ScriptContext::UI)
{
	g_pSquirrel<ScriptContext::UI>->pushbool(sqvm, true /*g_pMasterServerManager->m_bHasMainMenuPromoData*/);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("var", NSGetCustomMainMenuPromoData, "int promoDataKey", "", ScriptContext::UI)
{
	int nPromoDataKey = g_pSquirrel<ScriptContext::UI>->getinteger(sqvm, 1);
	switch (g_pSquirrel<ScriptContext::UI>->getinteger(sqvm, 1))
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
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, "STUB");
		break;
	}
	case eMainMenuPromoDataProperty::smallButton1ImageIndex:
	{
		g_pSquirrel<ScriptContext::UI>->pushinteger(sqvm, 0);
		break;
	}
	case eMainMenuPromoDataProperty::smallButton2ImageIndex:
	{
		g_pSquirrel<ScriptContext::UI>->pushinteger(sqvm, 1);
		break;
	}

	case eMainMenuPromoDataProperty::largeButtonImageIndex:
	{
		g_pSquirrel<ScriptContext::UI>->pushinteger(sqvm, 2);
		break;
	}
	}

	return SQRESULT_NOTNULL;
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsMasterServerAuthenticated, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pAtlasClient->GetOriginAuthSuccessful());
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSRequestServerList, "", "", ScriptContext::UI)
{
	g_pAtlasClient->FetchRemoteGameServerList();
	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSIsRequestingServerList, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pAtlasClient->GetFetchingRemoteGameServers());
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSMasterServerConnectionSuccessful, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, true /*g_pMasterServerManager->m_bSuccessfullyConnected*/);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("int", NSGetServerCount, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushinteger(sqvm, g_pAtlasClient->GetRemoteGameServerList().size());
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSClearRecievedServerList, "", "", ScriptContext::UI)
{
	g_pAtlasClient->ClearRemoteGameServerList();
	return SQRESULT_NULL;
}

// functions for authenticating with servers

ADD_SQFUNC("void", NSTryAuthWithServer, "int serverIndex, string password = ''", "", ScriptContext::UI)
{
	SQInteger serverIndex = g_pSquirrel<context>->getinteger(sqvm, 1);
	const SQChar* password = g_pSquirrel<context>->getstring(sqvm, 2);

	if (strncmp(g_pServerGlobalVariables->m_pMapName, "mp_lobby", 9))
	{
		g_pSquirrel<context>->raiseerror(sqvm, "Can only run NSTryAuthWithServer while in mp_lobby");
		return SQRESULT_ERROR;
	}

	int nNumServers = g_pAtlasClient->GetRemoteGameServerList().size();

	if (serverIndex >= nNumServers || serverIndex < 0)
	{
		g_pSquirrel<context>->raiseerror(sqvm, FormatA("Tried to auth with server index %i when only %i servers are available", serverIndex, nNumServers).c_str());
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

ADD_SQFUNC("bool", NSIsAuthenticatingWithServer, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, false /*g_pMasterServerManager->m_bScriptAuthenticatingWithGameServer*/);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSWasAuthSuccessful, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, true /*g_pMasterServerManager->m_bSuccessfullyAuthenticatedWithGameServer*/);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSConnectToAuthedServer, "", "", ScriptContext::UI)
{
	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSTryAuthWithLocalServer, "", "", ScriptContext::UI)
{
	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSCompleteAuthWithLocalServer, "", "", ScriptContext::UI)
{
	return SQRESULT_NULL;
}

ADD_SQFUNC("string", NSGetAuthFailReason, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushstring(sqvm, "STUB", -1);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("array<ServerInfo>", NSGetGameServers, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->newarray(sqvm, 0);
	size_t nIdx = 0;
	for (const RemoteGameServer_t& remoteServer : g_pAtlasClient->GetRemoteGameServerList())
	{
		g_pSquirrel<context>->pushnewstructinstance(sqvm, 11);

		// index
		g_pSquirrel<context>->pushinteger(sqvm, nIdx++);
		g_pSquirrel<context>->sealstructslot(sqvm, 0);

		// id
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.m_svID.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 1);

		// name
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.m_svName.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 2);

		// description
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.m_svDescription.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 3);

		// map
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.m_svMap.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 4);

		// playlist
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.m_svPlaylist.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 5);

		// playerCount
		g_pSquirrel<context>->pushinteger(sqvm, remoteServer.m_nPlayerCount);
		g_pSquirrel<context>->sealstructslot(sqvm, 6);

		// maxPlayerCount
		g_pSquirrel<context>->pushinteger(sqvm, remoteServer.m_nMaxPlayers);
		g_pSquirrel<context>->sealstructslot(sqvm, 7);

		// requiresPassword
		g_pSquirrel<context>->pushbool(sqvm, remoteServer.m_bRequiresPassword);
		g_pSquirrel<context>->sealstructslot(sqvm, 8);

		// region
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.m_svRegion.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 9);

		// requiredMods
		g_pSquirrel<context>->newarray(sqvm);
		for (const RemoteGameMod_t& mod : remoteServer.m_vRequiredMods)
		{
			g_pSquirrel<context>->pushnewstructinstance(sqvm, 2);

			// name
			g_pSquirrel<context>->pushstring(sqvm, mod.m_svName.c_str(), -1);
			g_pSquirrel<context>->sealstructslot(sqvm, 0);

			// version
			g_pSquirrel<context>->pushstring(sqvm, mod.m_svVersion.c_str(), -1);
			g_pSquirrel<context>->sealstructslot(sqvm, 1);

			g_pSquirrel<context>->arrayappend(sqvm, -2);
		}
		g_pSquirrel<context>->sealstructslot(sqvm, 10);

		g_pSquirrel<context>->arrayappend(sqvm, -2);
	}
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSPushGameStateData, "GameStateStruct gamestate", "", ScriptContext::CLIENT)
{
	SQStructInstance* structInst = g_pSquirrel<ScriptContext::CLIENT>->m_pSQVM->sqvm->_stackOfCurrentFunction[1]._VAL.asStructInstance;

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSPushUIPresence, "UIPresenceStruct presence", "", ScriptContext::UI)
{
	SQStructInstance* structInst = g_pSquirrel<ScriptContext::UI>->m_pSQVM->sqvm->_stackOfCurrentFunction[1]._VAL.asStructInstance;

	return SQRESULT_NOTNULL;
}
