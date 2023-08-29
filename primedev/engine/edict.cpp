#include "engine/edict.h"

ON_DLL_LOAD("engine.dll", EngineEdict, (CModule module))
{
	g_pServerGlobalVariables = module.Offset(0x7C6F70).RCast<CGlobalVars*>();
	g_pClientGlobalVariables = module.Offset(0x7A5C30).RCast<CGlobalVarsBase*>();
}
