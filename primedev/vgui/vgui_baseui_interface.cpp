#include "vgui_baseui_interface.h"
#include <mods/modmanager.h>

bool g_bEngineVguiInitilased = false;

// NOTE: CEngineVGui__Init is hooked in modlocalisation.cpp

void (*o_CEngineVGui__Shutdown)(void* self);

void h_CEngineVGui__Shutdown(void* self)
{
	g_bEngineVguiInitilased = false;
	o_CEngineVGui__Shutdown(self);
}

ON_DLL_LOAD_CLIENT("engine.dll", EngineVgui, (CModule module))
{
	o_CEngineVGui__Shutdown = module.Offset(0x249A70).RCast<void (*)(void*)>();

	HookAttach(&(PVOID&)o_CEngineVGui__Shutdown, (PVOID)h_CEngineVGui__Shutdown);
}
