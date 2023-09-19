#include "windows/wconsole.h"

HWND* g_hGameWindow = nullptr;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool (*o_Respawn_CreateWindow)(void* a1);
bool h_Respawn_CreateWindow(void* a1)
{
	Console_PostInit();
	return o_Respawn_CreateWindow(a1);
}

//-----------------------------------------------------------------------------
ON_DLL_LOAD_CLIENT("engine.dll", CreateWindowLog, (CModule module))
{
	o_Respawn_CreateWindow = module.Offset(0x1CD0E0).RCast<bool (*)(void*)>();
	HookAttach(&(PVOID&)o_Respawn_CreateWindow, (PVOID)h_Respawn_CreateWindow);

	g_hGameWindow = module.Offset(0x7d88a0).RCast<HWND*>();
}
