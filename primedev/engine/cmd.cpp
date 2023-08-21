#include "engine/cmd.h"

ON_DLL_LOAD("engine.dll", EngineCmd, (CModule module))
{
	Cbuf_GetCurrentPlayer = module.Offset(0x120630).RCast<ECommandTarget_t (*)()>();
	Cbuf_AddText = module.Offset(0x1203B0).RCast<void (*)(ECommandTarget_t eTarget, const char* text, cmd_source_t source)>();
	Cbuf_Execute = module.Offset(0x1204B0).RCast<void (*)()>();
}
