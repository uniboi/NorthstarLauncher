#include "squirrel/squirrelautobind.h"
#include "squirrel/squirrelclasstypes.h"
#include "squirrel/squirreldatatypes.h"
#include "squirrel/squirrel.h"

#include "client/localchatwriter.h"
#include "windows/window.h"
#include "networksystem/masterserver.h"
#include "server/auth/serverauthentication.h"
#include "shared/gamepresence.h"
#include "originsdk/origin.h"

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
	g_pMasterServerManager->RequestMainMenuPromos();
	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSHasCustomMainMenuPromoData, "", "", ScriptContext::UI)
{
	g_pSquirrel<ScriptContext::UI>->pushbool(sqvm, g_pMasterServerManager->m_bHasMainMenuPromoData);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("var", NSGetCustomMainMenuPromoData, "int promoDataKey", "", ScriptContext::UI)
{
	if (!g_pMasterServerManager->m_bHasMainMenuPromoData)
		return SQRESULT_NULL;

	switch (g_pSquirrel<ScriptContext::UI>->getinteger(sqvm, 1))
	{
	case eMainMenuPromoDataProperty::newInfoTitle1:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.newInfoTitle1.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::newInfoTitle2:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.newInfoTitle2.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::newInfoTitle3:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.newInfoTitle3.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::largeButtonTitle:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.largeButtonTitle.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::largeButtonText:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.largeButtonText.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::largeButtonUrl:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.largeButtonUrl.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::largeButtonImageIndex:
	{
		g_pSquirrel<ScriptContext::UI>->pushinteger(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.largeButtonImageIndex);
		break;
	}

	case eMainMenuPromoDataProperty::smallButton1Title:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.smallButton1Title.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::smallButton1Url:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.smallButton1Url.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::smallButton1ImageIndex:
	{
		g_pSquirrel<ScriptContext::UI>->pushinteger(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.smallButton1ImageIndex);
		break;
	}

	case eMainMenuPromoDataProperty::smallButton2Title:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.smallButton2Title.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::smallButton2Url:
	{
		g_pSquirrel<ScriptContext::UI>->pushstring(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.smallButton2Url.c_str());
		break;
	}

	case eMainMenuPromoDataProperty::smallButton2ImageIndex:
	{
		g_pSquirrel<ScriptContext::UI>->pushinteger(sqvm, g_pMasterServerManager->m_sMainMenuPromoData.smallButton2ImageIndex);
		break;
	}
	}

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsMasterServerAuthenticated, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pMasterServerManager->m_bOriginAuthWithMasterServerDone);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSRequestServerList, "", "", ScriptContext::UI)
{
	g_pMasterServerManager->RequestServerList();
	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSIsRequestingServerList, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pMasterServerManager->m_bScriptRequestingServerList);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSMasterServerConnectionSuccessful, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pMasterServerManager->m_bSuccessfullyConnected);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("int", NSGetServerCount, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushinteger(sqvm, g_pMasterServerManager->m_vRemoteServers.size());
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSClearRecievedServerList, "", "", ScriptContext::UI)
{
	g_pMasterServerManager->ClearServerList();
	return SQRESULT_NULL;
}

// functions for authenticating with servers

ADD_SQFUNC("void", NSTryAuthWithServer, "int serverIndex, string password = ''", "", ScriptContext::UI)
{
	SQInteger serverIndex = g_pSquirrel<context>->getinteger(sqvm, 1);
	const SQChar* password = g_pSquirrel<context>->getstring(sqvm, 2);

	if (serverIndex >= g_pMasterServerManager->m_vRemoteServers.size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("Tried to auth with server index {} when only {} servers are available", serverIndex, g_pMasterServerManager->m_vRemoteServers.size()).c_str());
		return SQRESULT_ERROR;
	}

	// send off persistent data first, don't worry about server/client stuff, since m_additionalPlayerData should only have entries when
	// we're a local server note: this seems like it could create a race condition, test later
	for (auto& pair : g_pServerAuthentication->m_PlayerAuthenticationData)
		g_pServerAuthentication->WritePersistentData(pair.first);

	// do auth
	g_pMasterServerManager->AuthenticateWithServer(g_pLocalPlayerUserID, g_pMasterServerManager->m_sOwnClientAuthToken, g_pMasterServerManager->m_vRemoteServers[serverIndex], (char*)password);

	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSIsAuthenticatingWithServer, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pMasterServerManager->m_bScriptAuthenticatingWithGameServer);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSWasAuthSuccessful, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushbool(sqvm, g_pMasterServerManager->m_bSuccessfullyAuthenticatedWithGameServer);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSConnectToAuthedServer, "", "", ScriptContext::UI)
{
	if (!g_pMasterServerManager->m_bHasPendingConnectionInfo)
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("Tried to connect to authed server before any pending connection info was available").c_str());
		return SQRESULT_ERROR;
	}

	RemoteServerConnectionInfo& info = g_pMasterServerManager->m_pendingConnectionInfo;

	// set auth token, then try to connect
	// i'm honestly not entirely sure how silentconnect works regarding ports and encryption so using connect for now
	g_pCVar->FindVar("serverfilter")->SetValue(info.authToken);
	Cbuf_AddText(Cbuf_GetCurrentPlayer(), fmt::format("connect {}.{}.{}.{}:{}", info.ip.S_un.S_un_b.s_b1, info.ip.S_un.S_un_b.s_b2, info.ip.S_un.S_un_b.s_b3, info.ip.S_un.S_un_b.s_b4, info.port).c_str(), cmd_source_t::kCommandSrcCode);

	g_pMasterServerManager->m_bHasPendingConnectionInfo = false;
	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSTryAuthWithLocalServer, "", "", ScriptContext::UI)
{
	// do auth request
	g_pMasterServerManager->AuthenticateWithOwnServer(g_pLocalPlayerUserID, g_pMasterServerManager->m_sOwnClientAuthToken);

	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSCompleteAuthWithLocalServer, "", "", ScriptContext::UI)
{
	// literally just set serverfilter
	// note: this assumes we have no authdata other than our own
	if (g_pServerAuthentication->m_RemoteAuthenticationData.size())
		g_pCVar->FindVar("serverfilter")->SetValue(g_pServerAuthentication->m_RemoteAuthenticationData.begin()->first.c_str());

	return SQRESULT_NULL;
}

ADD_SQFUNC("string", NSGetAuthFailReason, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->pushstring(sqvm, g_pMasterServerManager->m_sAuthFailureReason.c_str(), -1);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("array<ServerInfo>", NSGetGameServers, "", "", ScriptContext::UI)
{
	g_pSquirrel<context>->newarray(sqvm, 0);
	for (size_t i = 0; i < g_pMasterServerManager->m_vRemoteServers.size(); i++)
	{
		const RemoteServerInfo& remoteServer = g_pMasterServerManager->m_vRemoteServers[i];

		g_pSquirrel<context>->pushnewstructinstance(sqvm, 11);

		// index
		g_pSquirrel<context>->pushinteger(sqvm, i);
		g_pSquirrel<context>->sealstructslot(sqvm, 0);

		// id
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.id, -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 1);

		// name
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.name, -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 2);

		// description
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.description.c_str(), -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 3);

		// map
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.map, -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 4);

		// playlist
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.playlist, -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 5);

		// playerCount
		g_pSquirrel<context>->pushinteger(sqvm, remoteServer.playerCount);
		g_pSquirrel<context>->sealstructslot(sqvm, 6);

		// maxPlayerCount
		g_pSquirrel<context>->pushinteger(sqvm, remoteServer.maxPlayers);
		g_pSquirrel<context>->sealstructslot(sqvm, 7);

		// requiresPassword
		g_pSquirrel<context>->pushbool(sqvm, remoteServer.requiresPassword);
		g_pSquirrel<context>->sealstructslot(sqvm, 8);

		// region
		g_pSquirrel<context>->pushstring(sqvm, remoteServer.region, -1);
		g_pSquirrel<context>->sealstructslot(sqvm, 9);

		// requiredMods
		g_pSquirrel<context>->newarray(sqvm);
		for (const RemoteModInfo& mod : remoteServer.requiredMods)
		{
			g_pSquirrel<context>->pushnewstructinstance(sqvm, 2);

			// name
			g_pSquirrel<context>->pushstring(sqvm, mod.Name.c_str(), -1);
			g_pSquirrel<context>->sealstructslot(sqvm, 0);

			// version
			g_pSquirrel<context>->pushstring(sqvm, mod.Version.c_str(), -1);
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
	g_pGameStatePresence->map = structInst->data[0]._VAL.asString->_val;
	g_pGameStatePresence->mapDisplayname = structInst->data[1]._VAL.asString->_val;
	g_pGameStatePresence->playlist = structInst->data[2]._VAL.asString->_val;
	g_pGameStatePresence->playlistDisplayname = structInst->data[3]._VAL.asString->_val;

	g_pGameStatePresence->currentPlayers = structInst->data[4]._VAL.asInteger;
	g_pGameStatePresence->maxPlayers = structInst->data[5]._VAL.asInteger;
	g_pGameStatePresence->ownScore = structInst->data[6]._VAL.asInteger;
	g_pGameStatePresence->otherHighestScore = structInst->data[7]._VAL.asInteger;
	g_pGameStatePresence->maxScore = structInst->data[8]._VAL.asInteger;
	g_pGameStatePresence->timestampEnd = ceil(structInst->data[9]._VAL.asFloat);

	if (g_pMasterServerManager->m_currentServer)
	{
		g_pGameStatePresence->id = g_pMasterServerManager->m_currentServer->id;
		g_pGameStatePresence->name = g_pMasterServerManager->m_currentServer->name;
		g_pGameStatePresence->description = g_pMasterServerManager->m_currentServer->description;
		g_pGameStatePresence->password = g_pMasterServerManager->m_sCurrentServerPassword;
	}

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("void", NSPushUIPresence, "UIPresenceStruct presence", "", ScriptContext::UI)
{
	SQStructInstance* structInst = g_pSquirrel<ScriptContext::UI>->m_pSQVM->sqvm->_stackOfCurrentFunction[1]._VAL.asStructInstance;

	g_pGameStatePresence->isLoading = structInst->data[0]._VAL.asInteger;
	g_pGameStatePresence->isLobby = structInst->data[1]._VAL.asInteger;
	g_pGameStatePresence->loadingLevel = structInst->data[2]._VAL.asString->_val;
	g_pGameStatePresence->uiMap = structInst->data[3]._VAL.asString->_val;

	return SQRESULT_NOTNULL;
}
