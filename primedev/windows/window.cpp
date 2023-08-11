#include "windows/wconsole.h"

AUTOHOOK_INIT()

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
// clang-format off
AUTOHOOK(Respawn_CreateWindow, engine.dll + 0x1CD0E0, bool, __fastcall,
	(void* a1))
// clang-format on
{
	Console_PostInit();
	return Respawn_CreateWindow(a1);
}

//-----------------------------------------------------------------------------
ON_DLL_LOAD_CLIENT("engine.dll", CreateWindowLog, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
