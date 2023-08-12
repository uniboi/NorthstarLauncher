#include "vgui_baseui_interface.h"
#include <mods/modmanager.h>

bool g_bEngineVguiInitilased = false;

AUTOHOOK_INIT()

// NOTE: CEngineVGui__Init is hooked in modlocalisation.cpp

// clang-format off
AUTOHOOK(CEngineVGui__Shutdown, engine.dll + 0x249A70,
void, __fastcall, (void* self))
// clang-format on
{
	g_bEngineVguiInitilased = false;
	CEngineVGui__Shutdown(self);
}

ON_DLL_LOAD_CLIENT("engine.dll", EngineVgui, (CModule module))
{
	AUTOHOOK_DISPATCH()
}


