#include "tier1/convar.h"
#include "engine/r2engine.h"
#include "shared/exploit_fixes/ns_limits.h"
#include "masterserver/masterserver.h"
#include "networksystem/bcrypt.h"

#include <string>
#include <thread>

AUTOHOOK_INIT()

// v1 HMACSHA256-signed masterserver request (HMAC-SHA256(JSONData, MasterServerToken) + JSONData)
static void ProcessAtlasConnectionlessPacketSigreq1(netpacket_t* packet, bool dbg, std::string pType, std::string pData)
{
	if (pData.length() < HMACSHA256_LEN)
	{
		if (dbg)
			Warning(
				eLog::MS,
				"ignoring Atlas connectionless packet (size=%i type=%s): invalid: too short for signature\n",
				packet->size,
				pType.c_str());
		return;
	}

	std::string pSig; // is binary data, not actually an ASCII string
	pSig = pData.substr(0, HMACSHA256_LEN);
	pData = pData.substr(HMACSHA256_LEN);

	if (!g_pMasterServerManager || !g_pMasterServerManager->m_sOwnServerAuthToken[0])
	{
		if (dbg)
			Warning(
				eLog::MS,
				"ignoring Atlas connectionless packet (size=%i type=%s): invalid (data=%s): no masterserver token yet\n",
				packet->size,
				pType.c_str(),
				pData.c_str());
		return;
	}

	if (!VerifyHMACSHA256(std::string(g_pMasterServerManager->m_sOwnServerAuthToken), pSig, pData))
	{
		if (!Cvar_net_debug_atlas_packet_insecure->GetBool())
		{
			if (dbg)
				Warning(
					eLog::MS,
					"ignoring Atlas connectionless packet (size=%i type=%s): invalid: invalid signature (key=%s)\n",
					packet->size,
					pType.c_str(),
					g_pMasterServerManager->m_sOwnServerAuthToken);
			return;
		}
		Warning(
			eLog::MS,
			"processing Atlas connectionless packet (size=%i type=%s) with invalid signature due to net_debug_atlas_packet_insecure\n",
			packet->size,
			pType.c_str());
	}

	if (dbg)
		DevMsg(eLog::MS, "got Atlas connectionless packet (size=%i type=%s data=%s)\n", packet->size, pType.c_str(), pData.c_str());

	std::thread t(&MasterServerManager::ProcessConnectionlessPacketSigreq1, g_pMasterServerManager, pData);
	t.detach();

	return;
}

static void ProcessAtlasConnectionlessPacket(netpacket_t* packet)
{
	bool dbg = Cvar_net_debug_atlas_packet->GetBool();

	// extract kind, null-terminated type, data
	std::string pType, pData;
	for (int i = 5; i < packet->size; i++)
	{
		if (packet->data[i] == '\x00')
		{
			pType.assign((char*)(&packet->data[5]), (size_t)(i - 5));
			if (i + 1 < packet->size)
				pData.assign((char*)(&packet->data[i + 1]), (size_t)(packet->size - i - 1));
			break;
		}
	}

	// note: all Atlas connectionless packets should be idempotent so multiple attempts can be made to mitigate packet loss
	// note: all long-running Atlas connectionless packet handlers should be started in a new thread (with copies of the data) to avoid
	// blocking networking

	// v1 HMACSHA256-signed masterserver request
	if (pType == "sigreq1")
	{
		ProcessAtlasConnectionlessPacketSigreq1(packet, dbg, pType, pData);
		return;
	}

	if (dbg)
		Warning(eLog::MS, "ignoring Atlas connectionless packet (size=%i type=%s): unknown type\n", packet->size, pType.c_str());
	return;
}

AUTOHOOK(ProcessConnectionlessPacket, engine.dll + 0x117800, bool, , (void* a1, netpacket_t* packet))
{
	// packet->data consists of 0xFFFFFFFF (int32 -1) to indicate packets aren't split, followed by a header consisting of a single
	// character, which is used to uniquely identify the packet kind. Most kinds follow this with a null-terminated string payload
	// then an arbitrary amoount of data.

	// T (no rate limits since we authenticate packets before doing anything expensive)
	if (4 < packet->size && packet->data[4] == 'T')
	{
		ProcessAtlasConnectionlessPacket(packet);
		return false;
	}

	// check rate limits for the original unconnected packets
	if (!g_pServerLimits->CheckConnectionlessPacketLimits(packet))
		return false;

	// A, H, I, N
	return ProcessConnectionlessPacket(a1, packet);
}

ON_DLL_LOAD_RELIESON("engine.dll", ServerNetHooks, ConVar, (CModule module))
{
	AUTOHOOK_DISPATCH_MODULE(engine.dll)
}
