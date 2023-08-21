#include "r2engine.h"

CClient* g_pClientArray;

server_state_t* g_pServerState;

char* g_pModName = nullptr; // we cant set this up here atm since we dont have an offset to it in engine, instead we store it in IsRespawnMod

CGlobalVars* g_pServerGlobalVariables;
CGlobalVarsBase* g_pClientGlobalVariables;

ON_DLL_LOAD("engine.dll", R2Engine, (CModule module))
{
	g_pClientArray = module.Offset(0x12A53F90).RCast<CClient*>();

	g_pServerState = module.Offset(0x12A53D48).RCast<server_state_t*>();

	g_pServerGlobalVariables = module.Offset(0x7C6F70).RCast<CGlobalVars*>();
	g_pClientGlobalVariables = module.Offset(0x7A5C30).RCast<CGlobalVarsBase*>();
}
