#include "engine/sys_engine.h"

#include "hoststate.h"

void (*o_CEngine__Frame)(CEngine* self);

void h_CEngine__Frame(CEngine* self)
{
	o_CEngine__Frame(self);
}

ON_DLL_LOAD("engine.dll", RunFrame, (CModule module))
{
	o_CEngine__Frame = module.Offset(0x1C8650).RCast<void (*)(CEngine*)>();
	HookAttach(&(PVOID&)o_CEngine__Frame, (PVOID)h_CEngine__Frame);

	g_pEngine = module.Offset(0x7D70C8).Deref().RCast<CEngine*>();
}
