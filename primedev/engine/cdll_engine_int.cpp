#include "engine/cdll_engine_int.h"

ON_DLL_LOAD_CLIENT("engine.dll", CDllEngineInt, (CModule module))
{
	CEngineClient__Saytext = module.Offset(0x54780).RCast<void (*)(void*, const char*, uint64_t, bool)>();
}
