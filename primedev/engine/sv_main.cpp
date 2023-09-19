#include "networksystem/atlas.h"
#include "originsdk/origin.h"
#include "engine/edict.h"

bool (*o_SV_ActivateServer)();

bool h_SV_ActivateServer()
{
	return o_SV_ActivateServer();
}

ON_DLL_LOAD("engine.dll", EngineSvMain, (CModule module))
{
	o_SV_ActivateServer = module.Offset(0x108270).RCast<bool (*)()>();
	HookAttach(&(PVOID&)o_SV_ActivateServer, (PVOID)h_SV_ActivateServer);
}
