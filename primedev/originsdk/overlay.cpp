#include "originsdk/overlay.h"

ON_DLL_LOAD("engine.dll", OriginOverlay, (CModule module))
{
	g_pbOriginOverlayEnabled = module.Offset(0x13978255).RCast<bool*>();
}
