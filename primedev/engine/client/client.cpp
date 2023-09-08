#include "engine/client/client.h"

#include "networksystem/bansystem.h"
#include "networksystem/atlas.h"
#include "shared/exploit_fixes/ns_limits.h"

AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(CClient__Connect, engine.dll + 0x101740,
bool,, (CClient* self, char* pszName, void* pNetChannel, char bFakePlayer, void* a5, char pDisconnectReason[256], int nSize))
// clang-format on
{
	if (!CClient__Connect(self, pszName, pNetChannel, bFakePlayer, a5, pDisconnectReason, nSize))
		return false;

	// Set clantag for fake players
	if (bFakePlayer)
	{
		strncpy_s(self->m_szClanTag, 16, "BOT", 15);
		self->m_nPersistenceState = ePersistenceReady::READY;
	}

	return true;
}

// clang-format off
AUTOHOOK(CClient__ActivatePlayer, engine.dll + 0x100F80,
void,, (CClient* self))
// clang-format on
{
	CClient__ActivatePlayer(self);
}

// clang-format off
AUTOHOOK(_CClient__Disconnect, engine.dll + 0x1012C0,
void,, (CClient* self, uint32_t unknownButAlways1, const char* pReason, ...))
// clang-format on
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

	_CClient__Disconnect(self, unknownButAlways1, buf);
}

ON_DLL_LOAD("engine.dll", EngineClient, (CModule module))
{
	AUTOHOOK_DISPATCH()

	CClient__Disconnect = module.Offset(0x1012C0).RCast<void (*)(void*, uint32_t, const char*, ...)>();
}
