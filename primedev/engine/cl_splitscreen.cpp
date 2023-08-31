#include "engine/cl_splitscreen.h"

ON_DLL_LOAD("engine.dll", CLSplitScreen, (CModule module))
{
	GetBaseLocalClient = module.Offset(0x78200).RCast<void* (*)()>();
}
