#include "engine/client/client.h"

#include "networksystem/atlas.h"
#include "shared/exploit_fixes/ns_limits.h"
#include "engine/edict.h"

bool (*o_CClient__Connect)(CClient* self, char* pszName, void* pNetChannel, bool bFakePlayer, void* a5, char pDisconnectReason[256], int nSize);

bool h_CClient__Connect(CClient* self, char* pszName, void* pNetChannel, bool bFakePlayer, void* a5, char pDisconnectReason[256], int nSize)
{
	if (!o_CClient__Connect(self, pszName, pNetChannel, bFakePlayer, a5, pDisconnectReason, nSize))
		return false;

	// Set clantag for fake players
	if (bFakePlayer)
	{
		strncpy_s(self->m_szClanTag, 16, "BOT", 15);
		self->m_nPersistenceState = ePersistenceReady::READY;
	}

	return true;
}

void (*o_CClient__ActivatePlayer)(CClient* self);

void h_CClient__ActivatePlayer(CClient* self)
{
	o_CClient__ActivatePlayer(self);
}

void (*o_CClient__Disconnect)(CClient* self, uint32_t unknownButAlways1, const char* pReason, ...);

void CClient__Disconnect(CClient* self, uint32_t unknownButAlways1, const char* pReason, ...)
{
	// have to manually format message because can't pass varargs to original func
	char buf[1024];

	va_list va;
	va_start(va, pReason);
	vsprintf(buf, pReason, va);
	va_end(va);

	// this reason is used while connecting to a local server, hacky, but just ignore it
	if (strcmp(pReason, "Connection closing"))
	{
		DevMsg(eLog::NS, "Player %s disconnected: \"%s\"\n", self->m_szServerName, buf);

		// Only try to push if we're registered
		// This gets called when client is leaving their own lobby so this
		// is just a sanity check as we would fail to push most likely
		if (g_pAtlasServer->IsRegistered())
		{
			g_pAtlasServer->PushPersistence(self);
		}

		g_pServerLimits->RemovePlayer(self);
	}

	o_CClient__Disconnect(self, unknownButAlways1, "%s", buf);
}

size_t __fastcall ShouldAllowAlltalk()
{
	// this needs to return a 64 bit integer where 0 = true and 1 = false
	if (Cvar_sv_alltalk->GetBool())
		return 0;

	// lobby should default to alltalk, otherwise don't allow it
	return strcmp(g_pServerGlobalVariables->m_pMapName, "mp_lobby");
}

ON_DLL_LOAD("engine.dll", EngineClient, (CModule module))
{
	o_CClient__Connect = module.Offset(0x101740).RCast<bool (*)(CClient*, char*, void*, bool, void* a5, char[256], int)>();
	HookAttach(&(PVOID&)o_CClient__Connect, (PVOID)h_CClient__Connect);

	o_CClient__ActivatePlayer = module.Offset(0x100F80).RCast<void (*)(CClient*)>();
	HookAttach(&(PVOID&)o_CClient__ActivatePlayer, (PVOID)h_CClient__ActivatePlayer);

	o_CClient__Disconnect = module.Offset(0x1012C0).RCast<void (*)(CClient*, uint32_t, const char*, ...)>();
	HookAttach(&(PVOID&)o_CClient__Disconnect, (PVOID)CClient__Disconnect);

		// replace strcmp function called in CClient::ProcessVoiceData with our own code that calls ShouldAllowAllTalk
	CMemory base = module.Offset(0x1085FA);

	base.Patch("48 B8"); // mov rax, 64 bit int
	// (uint8_t*)&ShouldAllowAlltalk doesn't work for some reason? need to make it a uint64 first
	uint64_t pShouldAllowAllTalk = reinterpret_cast<uint64_t>(ShouldAllowAlltalk);
	base.Offset(0x2).Patch((uint8_t*)&pShouldAllowAllTalk, 8);
	base.Offset(0xA).Patch("FF D0"); // call rax

	// nop until compare (test eax, eax)
	base.Offset(0xC).NOP(0x7);
}
