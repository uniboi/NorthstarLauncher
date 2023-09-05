#include "networksystem/atlas.h"
#include "originsdk/origin.h"
#include "engine/edict.h"

AUTOHOOK_INIT()

AUTOHOOK(SV_ActivateServer, engine.dll + 0x108270, bool, __fastcall, ())
{
	return SV_ActivateServer();
}

ON_DLL_LOAD("engine.dll", EngineSvMain, (CModule modulle))
{
	AUTOHOOK_DISPATCH()
}
