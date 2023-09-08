#pragma once

#include "networksystem/netchannel.h"
#include "engine/client/client.h"

//-----------------------------------------------------------------------------
//
struct RemoteGameMod_t
{
	std::string m_svName;
	std::string m_svVersion;
};

//-----------------------------------------------------------------------------
//
struct RemoteGameServer_t
{
	std::string m_svID; // 32 bytes + nullterminator

	// server info
	std::string m_svName; // 64
	std::string m_svDescription;
	std::string m_svMap; // 32
	std::string m_svPlaylist; // 16
	std::string m_svRegion; // 32
	std::vector<RemoteGameMod_t> m_vRequiredMods;

	int m_nPlayerCount;
	int m_nMaxPlayers;

	bool m_bRequiresPassword;
};

//-----------------------------------------------------------------------------
//
class CAtlasClient
{
	friend class CAtlasServer;

  public:
	void AuthenticateOrigin(const char* pszUID, const char* pszToken);

	void ClearRemoteGameServerList();
	void FetchRemoteGameServerList();
	std::vector<RemoteGameServer_t> GetRemoteGameServerList();

	void AuthenticateRemoteGameServer(const char* pszUID, const char* pszPassword, RemoteGameServer_t server);

	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	bool GetOriginAuthInProgress() const
	{
		return m_bOriginAuthInProgress;
	}

	void SetOriginAuthInProgress(bool bState)
	{
		m_bOriginAuthInProgress = bState;
	}

	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	bool GetOriginAuthSuccessful() const
	{
		return m_bOriginAuthSuccessful;
	}

	void SetOriginAuthSuccessful(bool bState)
	{
		m_bOriginAuthSuccessful = bState;
	}

	//-----------------------------------------------------------------------------
	// Purpose:
	//-----------------------------------------------------------------------------
	bool GetFetchingRemoteGameServers() const
	{
		return m_bFetchingRemoteGameServers;
	}

	void SetFetchingRemoteGameServers(bool bState)
	{
		m_bFetchingRemoteGameServers = bState;
	}

  private:
	std::string m_svToken = "";

	bool m_bOriginAuthInProgress = false;
	bool m_bOriginAuthSuccessful = false;

	std::vector<RemoteGameServer_t> m_vServers;
	bool m_bFetchingRemoteGameServers = false;

	bool m_bAuthenticatingWithGameServer = false;
};

inline CAtlasClient* g_pAtlasClient = nullptr;

//-----------------------------------------------------------------------------
//
struct AuthInfo_t
{
	AuthInfo_t() : m_svName(), m_svUID(), m_svPData(), m_bValid(false) {}

	std::string m_svName;
	std::string m_svUID;

	std::string m_svPData;

	bool m_bValid;
};

//-----------------------------------------------------------------------------
//
class CAtlasServer
{
	friend class CAtlasClient;

  public:
	void HeartBeat(double flCurrentTime);
	void RegisterSelf();
	void UnregisterSelf();

	void HandleConnectionlessPacket(netpacket_t* packet);

	void AuthenticateLocalClient(std::string svUID);

	bool SetupClient(CClient* pClient, std::string svToken);
	void PushPersistence(CClient* pClient);

	bool HasAuthInfo(std::string svToken);
	AuthInfo_t GetAuthInfo(std::string svToken);
	void AddAuthInfo(std::string svToken, AuthInfo_t info);
	void RemoveAuthInfo(std::string svToken);
	void ClearAuthInfo();

	inline bool IsRegistered() const
	{
		return !m_svID.empty();
	}

  private:
	double m_flLastHearBeat = 0.0;

	bool m_bAttemptingToRegisterSelf = false;
	bool m_bSuccesfullyRegisteredSelf = false;

	std::string m_svID;
	std::string m_svAuthToken;

	// Map of auth info ( token : info )
	std::map<std::string, AuthInfo_t> m_mpAuthInfo;
	std::mutex m_AuthDataMutex;

	std::atomic_int m_iPersistencePushes = 0;
};

inline CAtlasServer* g_pAtlasServer = nullptr;
