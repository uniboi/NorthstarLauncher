#include "engine/cdll_engine_int.h"

#include "originsdk/overlay.h"

AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(CEngineClient__OpenExternalWebBrowser, engine.dll + 0x53B70, 
void, __fastcall, (void* thisptr, char* pUrl, char flags))
// clang-format on
{
	bool bIsOriginOverlayEnabledOriginal = *g_pbOriginOverlayEnabled;
	if (flags & 2 && !strncmp(pUrl, "http", 4)) // custom force external browser flag
		*g_pbOriginOverlayEnabled = false; // if this bool is false, game will use an external browser rather than the origin overlay one

	CEngineClient__OpenExternalWebBrowser(thisptr, pUrl, flags);
	*g_pbOriginOverlayEnabled = bIsOriginOverlayEnabledOriginal;
}

ON_DLL_LOAD_CLIENT("engine.dll", CDllEngineInt, (CModule module))
{
	AUTOHOOK_DISPATCH()

	CEngineClient__Saytext = module.Offset(0x54780).RCast<void (*)(void*, const char*, uint64_t, bool)>();
}
