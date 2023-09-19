#include "engine/cdll_engine_int.h"

#include "originsdk/overlay.h"

void (*o_CEngineClient__OpenExternalWebBrowser)(void* self, char* pUrl, char flags);

// clang-format off
void h_CEngineClient__OpenExternalWebBrowser(void* thisptr, char* pUrl, char flags)
// clang-format on
{
	bool bIsOriginOverlayEnabledOriginal = *g_pbOriginOverlayEnabled;
	if (flags & 2 && !strncmp(pUrl, "http", 4)) // custom force external browser flag
		*g_pbOriginOverlayEnabled = false; // if this bool is false, game will use an external browser rather than the origin overlay one

	o_CEngineClient__OpenExternalWebBrowser(thisptr, pUrl, flags);
	*g_pbOriginOverlayEnabled = bIsOriginOverlayEnabledOriginal;
}

ON_DLL_LOAD_CLIENT("engine.dll", CDllEngineInt, (CModule module))
{
	o_CEngineClient__OpenExternalWebBrowser = module.Offset(0x53B70).RCast<void (*)(void*, char*, char)>();
	HookAttach(&(PVOID&)o_CEngineClient__OpenExternalWebBrowser, (PVOID)h_CEngineClient__OpenExternalWebBrowser);

	CEngineClient__Saytext = module.Offset(0x54780).RCast<void (*)(void*, const char*, uint64_t, bool)>();
}
