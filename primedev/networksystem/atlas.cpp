#include "networksystem/atlas.h"

#include "tier2/curlutils.h"
#include "engine/edict.h"
#include "shared/playlist.h"
#include "engine/server/server.h"
#include "networksystem/bcrypt.h"
#include "networksystem/bansystem.h"
#include "mods/modmanager.h"

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasClient::AuthenticateOrigin(const char* pszUID, const char* pszToken)
{
	if (!pszUID || !pszToken)
	{
		Error(eLog::MS, NO_ERROR, "%s: pszUID or pszToken are NULL!\n", __FUNCTION__);
		return;
	}

	if (Cvar_ns_has_agreed_to_send_token->GetInt() != 1 /*AGREED_TO_SEND_TOKEN*/)
	{
		Error(eLog::MS, NO_ERROR, "%s: ns_has_agreed_to_send_token != AGREED_TO_SEND_TOKEN!\n", __FUNCTION__);
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
				Error(eLog::MS, NO_ERROR, "Failed performing northstar origin auth: '%s'\n", curl_easy_strerror(result));
				SetOriginAuthInProgress(false);
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "Response body is empty\n");
				SetOriginAuthInProgress(false);
				return;
			}

			DevMsg(eLog::MS, "%s\n", svResponse.c_str());

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				// We failed to auth for some reason, print it and return
				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "Failed to auth with atlas!\n");
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					SetOriginAuthInProgress(false);
					return;
				}

				SetOriginAuthSuccessful(true);
				m_svToken = jsResponse["token"].get<std::string>();
				DevMsg(eLog::MS, "Successfully authenticated with atlas!\n");
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "Failed parsing response json: '%s'\n", ex.what());
			}

			SetOriginAuthInProgress(false);
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasClient::ClearRemoteGameServerList()
{
	m_vServers.clear();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasClient::FetchRemoteGameServerList()
{
	if (GetFetchingRemoteGameServers())
	{
		Error(eLog::MS, NO_ERROR, "Already fetching the server list!\n");
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
				Error(eLog::MS, NO_ERROR, "Failed to fetch server list: '%s'\n", curl_easy_strerror(result));
				SetFetchingRemoteGameServers(false);
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "Server fetch response body is empty\n");
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

				DevMsg(eLog::MS, "Successfully fetched server list!\n");
			}
			catch (const std::exception& ex)
			{
				ClearRemoteGameServerList();
				Error(eLog::MS, NO_ERROR, "Failed parsing response json: '%s'\n", ex.what());
			}

			SetFetchingRemoteGameServers(false);
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
std::vector<RemoteGameServer_t> CAtlasClient::GetRemoteGameServerList()
{
	return m_vServers;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasClient::AuthenticateRemoteGameServer(const char* pszUID, const char* pszPassword, RemoteGameServer_t server)
{
	if (!pszUID || !pszPassword)
	{
		Error(eLog::MS, NO_ERROR, "pszUID or pszPassword are NULL!\n");
		return;
	}

	if (!GetOriginAuthSuccessful())
	{
		Error(eLog::MS, NO_ERROR, "Tried to connect to a server while not being authed with origin!\n");
		return;
	}

	if (m_bAuthenticatingWithGameServer)
	{
		Error(eLog::MS, NO_ERROR, "Already trying to authenticate with a server!\n");
		return;
	}

	m_bAuthenticatingWithGameServer = true;

	std::string svUID = pszUID;
	std::string svPassword = pszPassword;

	std::thread(
		[this, svUID, svPassword, server]()
		{
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
				Error(eLog::MS, NO_ERROR, "Failed to auth with server: '%s'\n", curl_easy_strerror(result));
				m_bAuthenticatingWithGameServer = false;
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "Server auth response body is empty\n");
				m_bAuthenticatingWithGameServer = false;
				return;
			}

			DevMsg(eLog::MS, "%s\n", svResponse.c_str());

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "Failed to auth with remote game server!\n");
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

				DevMsg(eLog::MS, "Successfully authed with server!\n");
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "Failed parsing response json: '%s'\n", ex.what());
			}

			m_bAuthenticatingWithGameServer = false;
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::HeartBeat(double flCurrentTime)
{
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

	// DevMsg(eLog::MS, "Running heartbeat\n");
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
				Error(eLog::MS, NO_ERROR, "Failed to submit server heartbeat: '%s'\n", curl_easy_strerror(nResult));
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "Server heartbeat response body is empty\n");
				return;
			}

			// DevMsg(eLog::MS, "%s\n", svResponse.c_str());

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "Failed heartbeat!\n");
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					return;
				}

				// Update ID and Token
				m_svID = jsResponse["id"].get<std::string>();
				m_svAuthToken = jsResponse["serverAuthToken"].get<std::string>();

				// DevMsg(eLog::MS, "Successfully heartbeat server!\n");
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "Failed parsing response json: '%s'\n", ex.what());
			}
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::RegisterSelf()
{
	if (m_bAttemptingToRegisterSelf)
	{
		return;
	}

	DevMsg(eLog::MS, "Attempting to register local server to atlas\n");
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
				Error(eLog::MS, NO_ERROR, "Failed to submit server registration: '%s'\n", curl_easy_strerror(nResult));
				m_bAttemptingToRegisterSelf = false;
				return;
			}

			if (svResponse.empty())
			{
				Error(eLog::MS, NO_ERROR, "Server registration response body is empty\n");
				m_bAttemptingToRegisterSelf = false;
				return;
			}

			DevMsg(eLog::MS, "%s\n", svResponse.c_str());

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "Failed to register self with atlas!\n");
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());

					m_bAttemptingToRegisterSelf = false;
					return;
				}

				m_svID = jsResponse["id"].get<std::string>();
				m_svAuthToken = jsResponse["serverAuthToken"].get<std::string>();

				DevMsg(eLog::MS, "Successfully registered server:\n");
				DevMsg(eLog::MS, "Name: %s\n", Cvar_hostname->GetString());
				DevMsg(eLog::MS, "ID  : %s\n", m_svID.c_str());
				m_bSuccesfullyRegisteredSelf = true;
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "Failed parsing response json: '%s'\n", ex.what());
			}

			m_bAttemptingToRegisterSelf = false;
		})
		.detach();
}

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
		}).detach();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::HandleConnectionlessPacket(netpacket_t* packet)
{
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
		Warning(eLog::MS, "Ignoring Atlas connectionless packet (size=%i type=%s): invalid: too short for signature", packet->size, pType.c_str());
		return;
	}

	std::string pSig; // is binary data, not actually an ASCII string
	pSig = pData.substr(0, HMACSHA256_LEN);
	pData = pData.substr(HMACSHA256_LEN);

	if (!g_pAtlasServer || !g_pAtlasServer->m_svAuthToken.size())
	{
		Warning(eLog::MS, "Ignoring Atlas connectionless packet (size=%i type=%s): invalid (data=%s): no masterserver token yet", packet->size, pType.c_str(), pData.c_str());
		return;
	}

	if (!VerifyHMACSHA256(g_pAtlasServer->m_svAuthToken, pSig, pData))
	{
		Warning(eLog::MS, "Ignoring Atlas connectionless packet (size=%i type=%s): invalid: invalid signature (key=%s)", packet->size, pType.c_str(), m_svAuthToken.c_str());
		return;
	}

	DevMsg(eLog::MS, "%s\n", pData.c_str());

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
					Error(eLog::MS, NO_ERROR, "Invalid atlas connectionless packet type!\n");
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
				// ip, time, token, type, uid, username
				// Each attempt has a unique token
			}
			catch (const std::exception& ex)
			{
				Error(eLog::MS, NO_ERROR, "Failed parsing atlas connectionless packet json!\n");
				return;
			}

			if (!svToken.size())
			{
				Error(eLog::MS, NO_ERROR, "svToken is empty!\n");
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
					Error(eLog::MS, NO_ERROR, "Failed to connect client to atlas: '%s'\n", curl_easy_strerror(nResult));
					return;
				}

				if (nResponse != 200)
				{
					Error(eLog::MS, NO_ERROR, "Failed making connect request: %ld\n", nResponse);
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
					Error(eLog::MS, NO_ERROR, "Persistence buffer too large!\n");
					return;
				}

				AuthInfo_t info;
				info.m_svName = svUserName;
				info.m_svUID = std::to_string(nUID);
				info.m_bValid = true;

				info.m_svPData = svResponse;

				DevMsg(eLog::MS, "Authenticated user '%s' ( %s ), pdata size: %li\n", info.m_svName.c_str(), info.m_svUID.c_str(), info.m_svPData.size());

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
					Error(eLog::MS, NO_ERROR, "Failed to reject connect request: '%s'\n", curl_easy_strerror(nResult));
					return;
				}

				if (nResponse != 200)
				{
					Error(eLog::MS, NO_ERROR, "Failed rejecting connect request: %ld\n", nResponse);
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

				DevMsg(eLog::MS, "Rejected atlas connectionless packet for client '%s' with reason: '%s'\n", svUserName.c_str(), svReject.c_str());
			}
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose: Authenticates local client, doesnt require port formard
// Note   : Not threaded off!!
//-----------------------------------------------------------------------------
void CAtlasServer::AuthenticateLocalClient(std::string svUID)
{
	DevMsg(eLog::MS, "Authenticating local client!\n");

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
		Error(eLog::MS, NO_ERROR, "Failed to reject connect request: '%s'\n", curl_easy_strerror(nResult));
		return;
	}

	try
	{
		nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

		if (jsResponse["success"] == false)
		{
			Error(eLog::MS, NO_ERROR, "Failed to submitting local auth request!\n");
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
			Error(eLog::MS, NO_ERROR, "Persistence array too large!\n");
			return;
		}

		info.m_svPData.resize(nSize);
		for (int i = 0; i < nSize; i++)
		{
			info.m_svPData[i] = jsResponse["persistentData"][i].get<int8_t>();
		}

		DevMsg(eLog::MS, "Authenticated local user ( %s ), pdata size: %li\n", info.m_svName.c_str(), info.m_svUID.c_str(), info.m_svPData.size());

		AddAuthInfo(svToken, info);

		g_pCVar->FindVar("serverFilter")->SetValue(svToken.c_str());
	}
	catch (const std::exception& ex)
	{
		Error(eLog::MS, NO_ERROR, "Failed parsing local client auth response: '%s'\n", ex.what());
	}
}

//-----------------------------------------------------------------------------
// Purpose:
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
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::PushPersistence(CClient* pClient)
{
	if (!pClient)
	{
		Error(eLog::NS, NO_ERROR, "Tried to push persistence for null client!\n");
		return;
	}

	if (pClient->m_nPersistenceState == ePersistenceReady::NOT_READY)
	{
		return;
	}

	DevMsg(eLog::MS, "Pushing persistence to atlas for: '%s' ( %s )\n", pClient->m_szServerName, pClient->m_UID);

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

			try
			{
				nlohmann::json jsResponse = nlohmann::json::parse(svResponse);

				if (jsResponse["success"] == false)
				{
					Error(eLog::MS, NO_ERROR, "Failed to register self with atlas!\n");
					if (jsResponse["error"]["enum"].is_string())
						Error(eLog::MS, NO_ERROR, "Code: '%s'\n", jsResponse["error"]["enum"].get<std::string>().c_str());
					if (jsResponse["error"]["msg"].is_string())
						Error(eLog::MS, NO_ERROR, "Msg : '%s'\n", jsResponse["error"]["msg"].get<std::string>().c_str());
				}
			}
			catch (const std::exception& ex)
			{
			}
		})
		.detach();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CAtlasServer::HasAuthInfo(std::string svToken)
{
	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	return m_mpAuthInfo.find(svToken) != m_mpAuthInfo.end();
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
AuthInfo_t CAtlasServer::GetAuthInfo(std::string svToken)
{
	if (!HasAuthInfo(svToken))
	{
		Error(eLog::MS, NO_ERROR, "Tried to get auth info for invalid token!\n");
		return AuthInfo_t(); // Return invalid auth info
	}

	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	return m_mpAuthInfo.at(svToken);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::AddAuthInfo(std::string svToken, AuthInfo_t info)
{
	if (HasAuthInfo(svToken))
	{
		// This sohuld never happen, let's log it though
		Error(eLog::MS, NO_ERROR, "Tried to add duplicate auth info!\n");
		return;
	}

	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	m_mpAuthInfo.insert(std::make_pair(svToken, info));
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::RemoveAuthInfo(std::string svToken)
{
	if (!HasAuthInfo(svToken))
	{
		// This sohuld never happen, let's log it though
		Error(eLog::MS, NO_ERROR, "Tried to remove non-existent auth info!\n");
		return;
	}

	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	m_mpAuthInfo.erase(svToken);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void CAtlasServer::ClearAuthInfo()
{
	std::lock_guard<std::mutex> guard(m_AuthDataMutex);

	m_mpAuthInfo.clear();
}
