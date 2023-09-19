#include "tier1/convar.h"
#include "originsdk/origin.h"
#include "networksystem/atlas.h"

char* g_pszNucleusToken;

// mirrored in script
const int NOT_DECIDED_TO_SEND_TOKEN = 0;
const int AGREED_TO_SEND_TOKEN = 1;
const int DISAGREED_TO_SEND_TOKEN = 2;

void (*o_AuthWithStryder)(void* a1);

// clang-format off
void h_AuthWithStryder(void* a1)
// clang-format on
{
	// We need to auth with atlas first
	if (Cvar_ns_has_agreed_to_send_token->GetInt() == AGREED_TO_SEND_TOKEN && !g_pAtlasClient->GetOriginAuthSuccessful())
	{
		g_pAtlasClient->AuthenticateOrigin(g_pLocalPlayerUserID, g_pLocalPlayerOriginToken);

		*g_pLocalPlayerOriginToken = 0;
	}

	o_AuthWithStryder(a1);
}

char* (*o_GetNucleusToken)();

char* h_GetNucleusToken()
{
	// Once we're authed with atlas clear this
	// The game sends this to game servers which is problematic
	// when it comes to community hosted ones
	if (g_pAtlasClient->GetOriginAuthSuccessful())
	{
		//memset(g_pszNucleusToken, 0x0, 1024);
		//strcpy(g_pszNucleusToken, "Protocol 3: Protect the Pilot");
	}

	return o_GetNucleusToken();
}

ON_DLL_LOAD_CLIENT("engine.dll", ClientAuthHooks, (CModule module))
{
	o_AuthWithStryder = module.Offset(0x1843A0).RCast<void (*)(void*)>();
	HookAttach(&(PVOID&)o_AuthWithStryder, (PVOID)h_AuthWithStryder);

	o_GetNucleusToken = module.Offset(0x183760).RCast<char* (*)()>();
	HookAttach(&(PVOID&)o_GetNucleusToken, (PVOID)h_GetNucleusToken);

	g_pszNucleusToken = module.Offset(0x13979D80).RCast<char*>();
}
