#include "engine/sys_engine.h"

#include "hoststate.h"
#include "server/serverpresence.h"

AUTOHOOK_INIT()

// clang-format off
AUTOHOOK(CEngine__Frame, engine.dll + 0x1C8650,
void, __fastcall, (CEngine* self))
// clang-format on
{
	CEngine__Frame(self);
}

ON_DLL_LOAD("engine.dll", RunFrame, (CModule module))
{
	AUTOHOOK_DISPATCH()

	g_pEngine = module.Offset(0x7D70C8).Deref().RCast<CEngine*>();
}
