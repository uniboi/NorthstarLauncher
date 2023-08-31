#include "game/client/hud_chat.h"

#include "game/server/gameinterface.h"
#include "squirrel/squirrel.h"

AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(CHudChat__AddGameLine, client.dll + 0x22E580,
void, __fastcall, (void* self, const char* message, int inboxId, bool isTeam, bool isDead))
// clang-format on
{
	// This hook is called for each HUD, but we only want our logic to run once.
	if (self != *CHudChat::allHuds)
		return;

	int senderId = inboxId & CUSTOM_MESSAGE_INDEX_MASK;
	bool isAnonymous = senderId == 0;
	bool isCustom = isAnonymous || (inboxId & CUSTOM_MESSAGE_INDEX_BIT);

	// Type is set to 0 for non-custom messages, custom messages have a type encoded as the first byte
	int type = 0;
	const char* payload = message;
	if (isCustom)
	{
		type = message[0];
		payload = message + 1;
	}

	RemoveAsciiControlSequences(const_cast<char*>(message), true);

	SQRESULT result = g_pSquirrel<ScriptContext::CLIENT>->Call("CHudChat_ProcessMessageStartThread", static_cast<int>(senderId) - 1, payload, isTeam, isDead, type);
	if (result == SQRESULT_ERROR)
		for (CHudChat* hud = *CHudChat::allHuds; hud != NULL; hud = hud->next)
			CHudChat__AddGameLine(hud, message, inboxId, isTeam, isDead);
}

ON_DLL_LOAD_CLIENT("client.dll", HudChat, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
