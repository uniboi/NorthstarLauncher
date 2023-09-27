#include "game/client/cdll_client_int.h"

void h_OnRenderStart()
{
	if (Cvar_r_latencyflex->GetBool() && LatencyFlex_WaitAndBeginFrame)
		LatencyFlex_WaitAndBeginFrame();

	o_OnRenderStart();
}

ON_DLL_LOAD_CLIENT("client.dll", LatencyFlex, (CModule module))
{
	o_OnRenderStart = module.Offset(0x1952C0).RCast<void (*)()>();
	HookAttach(&(PVOID&)o_OnRenderStart, (PVOID)h_OnRenderStart);

	DevMsg(eLog::NS, "LatencyFleX initialized.\n");
}
