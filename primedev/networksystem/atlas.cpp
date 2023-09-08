#include "networksystem/atlas.h"

#include "tier2/curlutils.h"
#include "engine/edict.h"
#include "shared/playlist.h"
#include "engine/server/server.h"
#include "networksystem/bcrypt.h"
#include "networksystem/bansystem.h"
#include "mods/modmanager.h"

// NOTE [Fifty]: Not using __FUNCTION__ as in threads it gets appended with more information
//               decreasing readabality

//-----------------------------------------------------------------------------
// Purpose: Authenticates the local client with stryder through atlas
// Input  : *pszUID - UID of the local client
//          *pszToken - Origin token of local client
//-----------------------------------------------------------------------------
void CAtlasClient::AuthenticateOrigin(const char* pszUID, const char* pszToken)
{
#define __FUNCTION "CAtlasClient::AuthenticateOrigin"
	if (!pszUID || !pszToken)
	{
		Error(eLog::MS, NO_ERROR, "%s: pszUID or pszToken are NULL!\n", __FUNCTION);
		return;
	}

	if (Cvar_ns_has_agreed_to_send_token->GetInt() != 1 /*AGREED_TO_SEND_TOKEN*/)
	{
		Error(eLog::MS, NO_ERROR, "%s: ns_has_agreed_to_send_token != AGREED_TO_SEND_TOKEN!\n", __FUNCTION);
		return;
	}

	if (GetOriginAuthInProgress())
	{
		return;
	}

	SetOriginAuthInProgress(true);

	std::string svUID = pszUID;
	std::string svToken = pszToken;

	std::thread(
		[this, svUID, svToken]()
		{
			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			std::string svUrl = FormatA("%s/client/origin_auth?id=%s&token=%s", Cvar_atlas_hostname->GetString(), svUID.c_str(), svToken.c_str());

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "GET", svResponse, cParms);

			CURLcode result = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			if (result != CURLcode::CURLE_OK)
			{
				Error(eLog::MS, NO_ERROR, "%s: Curl error: %s\n", __FUNCTION, curl_easy_strerror(result));
				SetOriginAuthInProgress(false);
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "%s: Response body is empty\n", __FUNCTION);
				SetOriginAuthInProgress(false);
				return;
			}

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				// We failed to auth for some reason, print it and return
				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed to authenticate with atlas!\n", __FUNCTION);
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					SetOriginAuthInProgress(false);
					return;
				}

				SetOriginAuthSuccessful(true);
				m_svToken = jsResponse["token"].get<std::string>();
				DevMsg(eLog::MS, "%s: Successfully authenticated with atlas!\n", __FUNCTION);
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "%s: Failed parsing response json: '%s'\n", __FUNCTION, ex.what());
			}

			SetOriginAuthInProgress(false);
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Clears the vector of remote servers
//-----------------------------------------------------------------------------
void CAtlasClient::ClearRemoteGameServerList()
{
	m_vServers.clear();
}

//-----------------------------------------------------------------------------
// Purpose: Fetches the list of remote servers from atlas
//-----------------------------------------------------------------------------
void CAtlasClient::FetchRemoteGameServerList()
{

#define __FUNCTION "CAtlasClient::FetchRemoteGameServerList"
	if (GetFetchingRemoteGameServers())
	{
		Error(eLog::MS, NO_ERROR, "%s: Already fetching the server list!\n", __FUNCTION);
		return;
	}

	SetFetchingRemoteGameServers(true);

	std::thread(
		[this]()
		{
			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			std::string svUrl = FormatA("%s/client/servers", Cvar_atlas_hostname->GetString());

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "GET", svResponse, cParms);

			CURLcode result = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			if (result != CURLcode::CURLE_OK)
			{
				Error(eLog::MS, NO_ERROR, "%s: Curl error: %s\n", __FUNCTION, curl_easy_strerror(result));
				SetFetchingRemoteGameServers(false);
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "%s: Response body is empty\n", __FUNCTION);
				SetFetchingRemoteGameServers(false);
				return;
			}

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				ClearRemoteGameServerList();

				for (auto& svr : jsResponse)
				{
					RemoteGameServer_t server;
					server.m_svID = svr.value("id", "");
					server.m_svName = svr.value("name", "");
					server.m_svDescription = svr.value("description", "");
					server.m_svMap = svr.value("map", "");
					server.m_svPlaylist = svr.value("playlist", "");
					server.m_svRegion = svr.value("region", "");

					for (auto& obj : svr["modInfo"]["Mods"])
					{
						RemoteGameMod_t mod;
						mod.m_svName = obj.value("Name", "");
						mod.m_svVersion = obj.value("Version", "");

						server.m_vRequiredMods.push_back(mod);
					}

					server.m_nPlayerCount = svr.value("playerCount", 0);
					server.m_nMaxPlayers = svr.value("maxPlayers", 0);
					server.m_bRequiresPassword = svr.value("hasPassword", false);

					m_vServers.push_back(server);
				}

				DevMsg(eLog::MS, "%s: Successfully fetched server list!\n", __FUNCTION);
			}
			catch (const std::exception& ex)
			{
				ClearRemoteGameServerList();
				Error(eLog::MS, NO_ERROR, "%s: Failed parsing response json: '%s'\n", ex.what(), __FUNCTION);
			}

			SetFetchingRemoteGameServers(false);
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Returns a vector of remote servers
//-----------------------------------------------------------------------------
std::vector<RemoteGameServer_t> CAtlasClient::GetRemoteGameServerList()
{
	return m_vServers;
}

//-----------------------------------------------------------------------------
// Purpose: Authenticates the client with a game server and joins when succesful
// Input  : *pszUID - UID of local client
//          *pszPassword - password passed by the client, empty if no password passed
//          server - The remote game server we want to authenticate and join
//-----------------------------------------------------------------------------
void CAtlasClient::AuthenticateRemoteGameServer(const char* pszUID, const char* pszPassword, RemoteGameServer_t server)
{
#define __FUNCTION "CAtlasClient::FetchRemoteGameServerList"
	if (!pszUID || !pszPassword)
	{
		Error(eLog::MS, NO_ERROR, "%s: pszUID or pszPassword are NULL!\n", __FUNCTION);
		return;
	}

	if (!GetOriginAuthSuccessful())
	{
		Error(eLog::MS, NO_ERROR, "%s: Tried to connect to a server while not being authed with origin!\n", __FUNCTION);
		return;
	}

	if (m_bAuthenticatingWithGameServer)
	{
		Error(eLog::MS, NO_ERROR, "%s: Already trying to authenticate with a server!\n", __FUNCTION);
		return;
	}

	m_bAuthenticatingWithGameServer = true;

	std::string svUID = pszUID;
	std::string svPassword = pszPassword;

	std::thread(
		[this, svUID, svPassword, server]()
		{
			double flStart = Plat_FloatTime();
			// First wait for our server to finish pushing all persistence, or timeout after 40 secs
			while (g_pAtlasServer->m_iPersistencePushes && flStart + 40.0 > Plat_FloatTime())
			{
				Sleep(100);
			}

			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			char* pszEscapedPassword = curl_easy_escape(nullptr, svPassword.c_str(), svPassword.length());
			std::string svUrl = FormatA("%s/client/auth_with_server?id=%s&playerToken=%s&server=%s&password=%s", Cvar_atlas_hostname->GetString(), svUID.c_str(), m_svToken.c_str(), server.m_svID.c_str(), pszEscapedPassword);
			curl_free(pszEscapedPassword);

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "POST", svResponse, cParms);

			CURLcode result = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			if (result != CURLcode::CURLE_OK)
			{
				Error(eLog::MS, NO_ERROR, "%s: Curl error %s\n", __FUNCTION, curl_easy_strerror(result));
				m_bAuthenticatingWithGameServer = false;
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "%s: Response body is empty\n", __FUNCTION);
				m_bAuthenticatingWithGameServer = false;
				return;
			}

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed to authenticate with remote game server!\n", __FUNCTION);
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					m_bAuthenticatingWithGameServer = false;
					return;
				}

				// "serverfilter" cvar is used to transfer the authToken from client to server, if it's bad client gets disconnected
				g_pCVar->FindVar("serverfilter")->SetValue(jsResponse["authToken"].get<std::string>().c_str());

				std::string svConnectCmd = FormatA("connect %s:%i", jsResponse["ip"].get<std::string>().c_str(), jsResponse["port"].get<int>());

				Cbuf_AddText(Cbuf_GetCurrentPlayer(), svConnectCmd.c_str(), cmd_source_t::kCommandSrcCode);

				DevMsg(eLog::MS, "%s: Successfully authed with server!\n", __FUNCTION);
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "%s: Failed parsing response json: '%s'\n", __FUNCTION, ex.what());
			}

			m_bAuthenticatingWithGameServer = false;
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Tries to send a heartbeat to atlas every 5 seconds only when
//          atlas_broadcast_local_server is true. If it is true and we're
//          not registered yet it registeres us with atlas
// Input  : flCurrentTime - Current time relative to program start
//-----------------------------------------------------------------------------
void CAtlasServer::HeartBeat(double flCurrentTime)
{
#define __FUNCTION "CAtlasServer::HeartBeat"
	// User doesnt want to breadcast, return
	if (!Cvar_atlas_broadcast_local_server->GetBool())
	{
		return;
	}

	// Only heartbeat every x seconds
	if (m_flLastHearBeat + 5.0 > flCurrentTime)
	{
		return;
	}

	m_flLastHearBeat = flCurrentTime;

	// If we're not registered yet try to do so
	if (!m_bSuccesfullyRegisteredSelf)
	{
		RegisterSelf();
		return;
	}

	std::thread(
		[this]()
		{
			// Format URL
			char* pszEscapedHostName = curl_easy_escape(nullptr, Cvar_hostname->GetString(), strlen(Cvar_hostname->GetString()));
			char* pszEscapedHostDescription = curl_easy_escape(nullptr, Cvar_hostdescription->GetString(), strlen(Cvar_hostdescription->GetString()));
			char* pszEscapedHostMap = curl_easy_escape(nullptr, g_pServerGlobalVariables->m_pMapName, strlen(g_pServerGlobalVariables->m_pMapName));
			char* pszEscapedHostPlaylist = curl_easy_escape(nullptr, Cvar_mp_gamemode->GetString(), strlen(Cvar_mp_gamemode->GetString()));
			char* pszEscapedHostPassword = curl_easy_escape(nullptr, Cvar_hostpassword->GetString(), strlen(Cvar_hostpassword->GetString()));
			std::string svUrl = FormatA("%s/server/update_values?id=%s&port=%i&authPort=udp&name=%s&description=%s&map=%s&playlist=%s&playerCount=%i&maxPlayers=%s&password=%s", Cvar_atlas_hostname->GetString(), m_svID.c_str(), Cvar_hostport->GetInt(),
										pszEscapedHostName, pszEscapedHostDescription, pszEscapedHostMap, pszEscapedHostPlaylist, g_pServer->GetNumClients(), R2::GetCurrentPlaylistVar("max_players", true), pszEscapedHostPassword);
			curl_free(pszEscapedHostName);
			curl_free(pszEscapedHostDescription);
			curl_free(pszEscapedHostMap);
			curl_free(pszEscapedHostPlaylist);
			curl_free(pszEscapedHostPassword);

			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			CURLMime cMime;
			cMime.svFileName = "modinfo.json";
			cMime.svName = "modinfo";
			cMime.svType = "application/json";

			// Build mods list
			nlohmann::json jsModList;
			for (const Mod& mod : g_pModManager->m_LoadedMods)
			{
				nlohmann::json jsMod;

				jsMod["Name"] = mod.Name;
				jsMod["Version"] = mod.Version;
				jsMod["RequiredOnClient"] = mod.RequiredOnClient;

				jsModList["Mods"].emplace_back(jsMod);
			}

			cMime.svData = jsModList.dump();

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "POST", svResponse, cParms, &cMime);

			CURLcode nResult = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			if (nResult != CURLcode::CURLE_OK)
			{
				Error(eLog::MS, NO_ERROR, "%s: Curl error %s\n", __FUNCTION, curl_easy_strerror(nResult));
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "%s: Response body is empty\n", __FUNCTION);
				return;
			}

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed heartbeat!\n", __FUNCTION);
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					return;
				}

				// Update ID and Token
				m_svID = jsResponse["id"].get<std::string>();
				m_svAuthToken = jsResponse["serverAuthToken"].get<std::string>();
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "%s: Failed parsing response json: '%s'\n", __FUNCTION, ex.what());
			}
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Registers us with atlas, only called from heartbeat
//-----------------------------------------------------------------------------
void CAtlasServer::RegisterSelf()
{
#define __FUNCTION "CAtlasServer::RegisterSelf"
	if (m_bAttemptingToRegisterSelf)
	{
		return;
	}

	DevMsg(eLog::MS, "%s: Attempting to register local server to atlas\n", __FUNCTION);
	m_bAttemptingToRegisterSelf = true;

	std::thread(
		[this]()
		{
			// Format URL
			char* pszEscapedHostName = curl_easy_escape(nullptr, Cvar_hostname->GetString(), strlen(Cvar_hostname->GetString()));
			char* pszEscapedHostDescription = curl_easy_escape(nullptr, Cvar_hostdescription->GetString(), strlen(Cvar_hostdescription->GetString()));
			char* pszEscapedHostMap = curl_easy_escape(nullptr, g_pServerGlobalVariables->m_pMapName, strlen(g_pServerGlobalVariables->m_pMapName));
			char* pszEscapedHostPlaylist = curl_easy_escape(nullptr, Cvar_mp_gamemode->GetString(), strlen(Cvar_mp_gamemode->GetString()));
			char* pszEscapedHostPassword = curl_easy_escape(nullptr, Cvar_hostpassword->GetString(), strlen(Cvar_hostpassword->GetString()));
			std::string svUrl = FormatA("%s/server/add_server?port=%i&authPort=udp&name=%s&description=%s&map=%s&playlist=%s&maxPlayers=%s&password=%s", Cvar_atlas_hostname->GetString(), Cvar_hostport->GetInt(), pszEscapedHostName,
										pszEscapedHostDescription, pszEscapedHostMap, pszEscapedHostPlaylist, R2::GetCurrentPlaylistVar("max_players", true), pszEscapedHostPassword);
			curl_free(pszEscapedHostName);
			curl_free(pszEscapedHostDescription);
			curl_free(pszEscapedHostMap);
			curl_free(pszEscapedHostPlaylist);
			curl_free(pszEscapedHostPassword);

			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			CURLMime cMime;
			cMime.svFileName = "modinfo.json";
			cMime.svName = "modinfo";
			cMime.svType = "application/json";

			// Build mods list
			nlohmann::json jsModList;
			for (const Mod& mod : g_pModManager->m_LoadedMods)
			{
				nlohmann::json jsMod;

				jsMod["Name"] = mod.Name;
				jsMod["Version"] = mod.Version;
				jsMod["RequiredOnClient"] = mod.RequiredOnClient;

				jsModList["Mods"].emplace_back(jsMod);
			}

			cMime.svData = jsModList.dump();

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "POST", svResponse, cParms, &cMime);

			CURLcode nResult = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			if (nResult != CURLcode::CURLE_OK)
			{
				Error(eLog::MS, NO_ERROR, "%s: Curl error: %s\n", __FUNCTION, curl_easy_strerror(nResult));
				m_bAttemptingToRegisterSelf = false;
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "%s: Response body is empty\n", __FUNCTION);
				m_bAttemptingToRegisterSelf = false;
				return;
			}

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed to register self with atlas!\n", __FUNCTION);
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					m_bAttemptingToRegisterSelf = false;
					return;
				}

				m_svID = jsResponse["id"].get<std::string>();
				m_svAuthToken = jsResponse["serverAuthToken"].get<std::string>();

				DevMsg(eLog::MS, "%s: Successfully registered server:\n", __FUNCTION);
				DevMsg(eLog::MS, "Name: %s\n", Cvar_hostname->GetString());
				DevMsg(eLog::MS, "ID  : %s\n", m_svID.c_str());
				m_bSuccesfullyRegisteredSelf = true;
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "%s: Failed parsing response json: '%s'\n", __FUNCTION, ex.what());
			}

			m_bAttemptingToRegisterSelf = false;
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Un-registers us from atlas
//-----------------------------------------------------------------------------
void CAtlasServer::UnregisterSelf()
{
	if (m_svAuthToken.empty())
	{
		return;
	}

	std::thread(
		[this]
		{
			std::string svUrl = FormatA("%s/server/remove_server?id=%s", Cvar_atlas_hostname->GetString(), m_svID.c_str());

			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "DELETE", svResponse, cParms);

			CURLcode nResult = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			m_svAuthToken = "";
			m_svID = "";
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose: Parses and responds to an atlas connectionless packet
// Input  : *packet -
//-----------------------------------------------------------------------------
void CAtlasServer::HandleConnectionlessPacket(netpacket_t* packet)
{
#define __FUNCTION "CAtlasServer::HandleConnectionlessPacket"
	if (!m_bSuccesfullyRegisteredSelf)
	{
		return;
	}

	std::string pType, pData;
	for (int i = 5; i < packet->size; i++)
	{
		if (packet->data[i] == '\x00')
		{
			pType.assign((char*)(&packet->data[5]), (size_t)(i - 5));
			if (i + 1 < packet->size)
			{
				pData.assign((char*)(&packet->data[i + 1]), (size_t)(packet->size - i - 1));
			}
			break;
		}
	}

	// Unsupported / Unknown type, return
	if (pType != "sigreq1")
	{
		return;
	}

	// v1 HMACSHA256-signed atlas request
	if (pData.length() < HMACSHA256_LEN)
	{
		Warning(eLog::MS, "%s: Ignoring Atlas connectionless packet (size=%i type=%s): invalid: too short for signature\n", __FUNCTION, packet->size, pType.c_str());
		return;
	}

	std::string pSig; // is binary data, not actually an ASCII string
	pSig = pData.substr(0, HMACSHA256_LEN);
	pData = pData.substr(HMACSHA256_LEN);

	if (!g_pAtlasServer || !g_pAtlasServer->m_svAuthToken.size())
	{
		Warning(eLog::MS, "%s: Ignoring Atlas connectionless packet (size=%i type=%s): invalid (data=%s): no masterserver token yet\n", __FUNCTION, packet->size, pType.c_str(), pData.c_str());
		return;
	}

	if (!VerifyHMACSHA256(g_pAtlasServer->m_svAuthToken, pSig, pData))
	{
		Warning(eLog::MS, "%s: Ignoring Atlas connectionless packet (size=%i type=%s): invalid: invalid signature (key=%s)\n", __FUNCTION, packet->size, pType.c_str(), m_svAuthToken.c_str());
		return;
	}

	// Don't block
	std::thread(
		[this, pData]()
		{
			std::string svToken;
			std::string svUserName;
			uint64_t nUID;
			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(pData);

				if (jsResponse["type"].get<std::string>() != "connect")
				{
					Error(eLog::MS, NO_ERROR, "%s: Invalid atlas connectionless packet type!\n", __FUNCTION);
					return;
				}

				svToken = jsResponse["token"].get<std::string>();
				svUserName = jsResponse["username"].get<std::string>();
				nUID = jsResponse["uid"].get<uint64_t>();

				// Already handled
				if (HasAuthInfo(svToken))
				{
					return;
				}

				// Each attempt has a unique token
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "%s: Failed parsing atlas connectionless packet json!\n", __FUNCTION);
				return;
			}

			if (!svToken.size())
			{
				Error(eLog::MS, NO_ERROR, "%s: svToken is empty!\n", __FUNCTION);
				return;
			}

			// We parsed the connectionless packet, lets send a response now
			std::string svReject = "";

			// if uid is banned reject the connecion
			if (!g_pBanSystem->IsUIDAllowed(nUID))
			{
				svReject = "Banned from server";
			}

			if (svReject == "")
			{
				std::string svUrl = FormatA("%s/server/connect?serverId=%s&token=%s", Cvar_atlas_hostname->GetString(), m_svID.c_str(), svToken.c_str());

				CURLParms cParms;
				cParms.nTimeout = 30; // TODO: make this a cvar
				cParms.pWriteFunc = CURLWriteStringCallback;
				cParms.bVerifyHost = true; // TODO: make this a cvar
				cParms.bVerifyPeer = true; // TODO: make this a cvar

				std::string svResponse;
				CURL* curl = CURLInitRequest(svUrl.c_str(), "GET", svResponse, cParms);

				CURLcode nResult = CURLSubmitRequest(curl);
				long nResponse = CURLGetResponse(curl);
				CURLCleanup(curl);

				if (nResult != CURLcode::CURLE_OK)
				{
					Error(eLog::MS, NO_ERROR, "%s (connect): Curl error: %s \n", __FUNCTION, curl_easy_strerror(nResult));
					return;
				}

				if (nResponse != 200)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed making connect request: %ld\n", __FUNCTION, nResponse);
					try
					{
						nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

						if (jsResponse["error"]["enum"].is_string())
							Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
						if (jsResponse["error"]["msg"].is_string())
							Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());
					}
					catch (const std::exception& ex)
					{
					}
					return;
				}
				if (svResponse.size() > PERSISTENCE_MAX_SIZE)
				{
					Error(eLog::MS, NO_ERROR, "%s: Persistence buffer too large!\n", __FUNCTION);
					return;
				}

				AuthInfo_t info;
				info.m_svName = svUserName;
				info.m_svUID = std::to_string(nUID);
				info.m_bValid = true;

				info.m_svPData = svResponse;

				DevMsg(eLog::MS, "%s: Authenticated user '%s' ( %s ), pdata size: %li\n", __FUNCTION, info.m_svName.c_str(), info.m_svUID.c_str(), info.m_svPData.size());

				AddAuthInfo(svToken, info);
			}
			else // Reject is not empty so lets reject the connect attempt
			{
				char* pszEscapedReject = curl_easy_escape(nullptr, svReject.c_str(), svReject.size());
				std::string svUrl = FormatA("%s/server/connect?serverId=%s&token=%s&reject=%s", Cvar_atlas_hostname->GetString(), m_svID.c_str(), svToken.c_str(), pszEscapedReject);
				curl_free(pszEscapedReject);

				CURLParms cParms;
				cParms.nTimeout = 30; // TODO: make this a cvar
				cParms.pWriteFunc = CURLWriteStringCallback;
				cParms.bVerifyHost = true; // TODO: make this a cvar
				cParms.bVerifyPeer = true; // TODO: make this a cvar

				std::string svResponse;
				CURL* curl = CURLInitRequest(svUrl.c_str(), "POST", svResponse, cParms);

				CURLcode nResult = CURLSubmitRequest(curl);
				long nResponse = CURLGetResponse(curl);
				CURLCleanup(curl);

				if (nResult != CURLcode::CURLE_OK)
				{
					Error(eLog::MS, NO_ERROR, "%s (reject): Curl error: %s\n", __FUNCTION, curl_easy_strerror(nResult));
					return;
				}

				if (nResponse != 200)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed rejecting connect request: %ld\n", __FUNCTION, nResponse);
					try
					{
						nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

						if (jsResponse["error"]["enum"].is_string())
							Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
						if (jsResponse["error"]["msg"].is_string())
							Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());
					}
					catch (const std::exception& ex)
					{
					}
					return;
				}

				DevMsg(eLog::MS, "%s: Rejected atlas connectionless packet for client '%s' with reason: '%s'\n", __FUNCTION, svUserName.c_str(), svReject.c_str());
			}
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Authenticates local client, doesnt require port formard
// Note   : Not threaded off!!
//-----------------------------------------------------------------------------
void CAtlasServer::AuthenticateLocalClient(std::string svUID)
{
#define __FUNCTION "CAtlasServer::AuthenticateLocalClient"
	DevMsg(eLog::MS, "%s: Authenticating local client!\n", __FUNCTION);

	std::string svUrl = FormatA("%s/client/auth_with_self?id=%s&playerToken=%s", Cvar_atlas_hostname->GetString(), svUID.c_str(), g_pAtlasClient->m_svToken.c_str());

	CURLParms cParms;
	cParms.nTimeout = 30; // TODO: make this a cvar
	cParms.pWriteFunc = CURLWriteStringCallback;
	cParms.bVerifyHost = true; // TODO: make this a cvar
	cParms.bVerifyPeer = true; // TODO: make this a cvar

	std::string svResponse;
	CURL* curl = CURLInitRequest(svUrl.c_str(), "POST", svResponse, cParms);

	CURLcode nResult = CURLSubmitRequest(curl);
	CURLCleanup(curl);

	if (nResult != CURLcode::CURLE_OK)
	{
		Error(eLog::MS, NO_ERROR, "%s: Curl error: '%s'\n", __FUNCTION, curl_easy_strerror(nResult));
		return;
	}

	try
	{
		nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

		if (jsResponse["success"] == false)
		{
			Error(eLog::MS, NO_ERROR, "%s: Failed to submitting local auth request!\n", __FUNCTION);
			if (jsResponse["error"]["enum"].is_string())
				Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
			if (jsResponse["error"]["msg"].is_string())
				Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());
			return;
		}

		std::string svToken = jsResponse["authToken"].get<std::string>();
		std::string svUID = jsResponse["id"].get<std::string>();

		AuthInfo_t info;
		info.m_svName = ""; // Local server
		info.m_svUID = svUID;
		info.m_bValid = true;

		int nSize = jsResponse["persistentData"].size();

		if (nSize > PERSISTENCE_MAX_SIZE)
		{
			Error(eLog::MS, NO_ERROR, "%s: Persistence array too large!\n", __FUNCTION);
			return;
		}

		info.m_svPData.resize(nSize);
		for (int i = 0; i < nSize; i++)
		{
			info.m_svPData[i] = jsResponse["persistentData"][i].get<int8_t>();
		}

		DevMsg(eLog::MS, "%s: Authenticated local user ( %s ), pdata size: %li\n", __FUNCTION, info.m_svName.c_str(), info.m_svUID.c_str(), info.m_svPData.size());

		AddAuthInfo(svToken, info);

		g_pCVar->FindVar("serverFilter")->SetValue(svToken.c_str());
	}
	catch (const std::exception& ex)
	{
		Error(eLog::MS, NO_ERROR, "%s: Failed parsing local client auth response: '%s'\n", __FUNCTION, ex.what());
	}
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Sets up a client setting its pdata
// Input  : *pClient -
//          svToken -
//-----------------------------------------------------------------------------
bool CAtlasServer::SetupClient(CClient* pClient, std::string svToken)
{
	AuthInfo_t info = GetAuthInfo(svToken);

	if (!info.m_bValid)
	{
		return false;
	}

	memcpy_s(pClient->m_PersistenceBuffer, PERSISTENCE_MAX_SIZE, info.m_svPData.data(), info.m_svPData.size());

	pClient->m_nPersistenceState = ePersistenceReady::READY_REMOTE;

	return true;
}

//-----------------------------------------------------------------------------
// Purpose: Pushes persistence of a client to atlas
//        : *pClient -
//-----------------------------------------------------------------------------
void CAtlasServer::PushPersistence(CClient* pClient)
{
#define __FUNCTION "CAtlasServer::PushPersistence"
	if (!pClient)
	{
		Error(eLog::NS, NO_ERROR, "%s: Tried to push persistence for null client!\n", __FUNCTION);
		return;
	}

	if (pClient->m_nPersistenceState == ePersistenceReady::NOT_READY)
	{
		return;
	}

	DevMsg(eLog::MS, "%s: Pushing persistence to atlas for: '%s' ( %s )\n", __FUNCTION, pClient->m_szServerName, pClient->m_UID);
	m_iPersistencePushes++;

	// FIXME [Fifty]: Atlas cries when i send pdata of PERSISTENCE_MAX_SIZE size
	//                '56306' is the size of pdata we get from atlas on auth so lets hope it doesnt break
	std::string pData;
	pData.resize(56306);
	memcpy_s(pData.data(), 56306, pClient->m_PersistenceBuffer, 56306);

	std::string svUID = pClient->m_UID;
	std::string svName = pClient->m_szServerName;

	std::thread(
		[this, svUID, svName, pData]()
		{
			CURLParms cParms;
			cParms.nTimeout = 30; // TODO: make this a cvar
			cParms.pWriteFunc = CURLWriteStringCallback;
			cParms.bVerifyHost = true; // TODO: make this a cvar
			cParms.bVerifyPeer = true; // TODO: make this a cvar

			CURLMime cMime;
			cMime.svData = pData;
			cMime.svFileName = "file.pdata";
			cMime.svName = "pdata";
			cMime.svType = "application/octet-stream";

			std::string svUrl = FormatA("%s/accounts/write_persistence?id=%s&serverId=%s", Cvar_atlas_hostname->GetString(), svUID.c_str(), m_svID.c_str());

			std::string svResponse;
			CURL* curl = CURLInitRequest(svUrl.c_str(), "POST", svResponse, cParms, &cMime);

			CURLcode nResult = CURLSubmitRequest(curl);
			CURLCleanup(curl);

			if (nResult != CURLcode::CURLE_OK)
			{
				Error(eLog::MS, NO_ERROR, "%s: Curl error: '%s'\n", __FUNCTION, curl_easy_strerror(nResult));
				m_iPersistencePushes--;
				return;
			}

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "%s: Failed to push persistence to atlas for '%s'!\n", __FUNCTION, svName.c_str());
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());
				}
			}
			catch (const std::exception& ex)
			{
			}

			m_iPersistencePushes--;
		})
		.detach();
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Checks if we have auth info for a token
// Input  : svToken -
//-----------------------------------------------------------------------------
bool CAtlasServer::HasAuthInfo(std::string svToken)
{
	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	return m_mpAuthInfo.find(svToken) != m_mpAuthInfo.end();
}

//-----------------------------------------------------------------------------
// Purpose: Gets the auth info for a token
// Input  : svToken -
//-----------------------------------------------------------------------------
AuthInfo_t CAtlasServer::GetAuthInfo(std::string svToken)
{
#define __FUNCTION "CAtlasServer::GetAuthInfo"
	if (!HasAuthInfo(svToken))
	{
		Error(eLog::MS, NO_ERROR, "%s: Tried to get auth info for invalid token!\n", __FUNCTION);
		return AuthInfo_t(); // Return invalid auth info
	}

	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	return m_mpAuthInfo.at(svToken);
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Adds auth info for a token
// Input  : svToken -
//          info -
//-----------------------------------------------------------------------------
void CAtlasServer::AddAuthInfo(std::string svToken, AuthInfo_t info)
{
#define __FUNCTION "CAtlasServer::AddAuthInfo"
	if (HasAuthInfo(svToken))
	{
		// This sohuld never happen, let's log it though
		Error(eLog::MS, NO_ERROR, "%s: Tried to add duplicate auth info!\n", __FUNCTION);
		return;
	}

	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	m_mpAuthInfo.insert(std::make_pair(svToken, info));
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Removes auth info for a token
// Input  : svToken -
//-----------------------------------------------------------------------------
void CAtlasServer::RemoveAuthInfo(std::string svToken)
{
#define __FUNCTION "CAtlasServer::RemoveAuthInfo"
	if (!HasAuthInfo(svToken))
	{
		// This sohuld never happen, let's log it though
		Error(eLog::MS, NO_ERROR, "%s: Tried to remove non-existent auth info!\n", __FUNCTION);
		return;
	}

	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	m_mpAuthInfo.erase(svToken);
#undef __FUNCTION
}

//-----------------------------------------------------------------------------
// Purpose: Clears the auth info map
//-----------------------------------------------------------------------------
void CAtlasServer::ClearAuthInfo()
{
	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	m_mpAuthInfo.clear();
}
