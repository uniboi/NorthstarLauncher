#include "engine/client/client.h"

ON_DLL_LOAD("engine.dll", EngineClient, (CModule module))
{
	CClient__Disconnect = module.Offset(0x1012C0).RCast<void (*)(void*, uint32_t, const char*, ...)>();
}
