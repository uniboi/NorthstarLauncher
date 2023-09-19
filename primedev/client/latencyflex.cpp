#include "tier1/convar.h"

void (*m_winelfx_WaitAndBeginFrame)();
void (*o_OnRenderStart)();

// clang-format off
void h_OnRenderStart()
// clang-format on
{
	if (Cvar_r_latencyflex->GetBool() && m_winelfx_WaitAndBeginFrame)
		m_winelfx_WaitAndBeginFrame();

	o_OnRenderStart();
}

ON_DLL_LOAD_CLIENT("client.dll", LatencyFlex, (CModule module))
{
	// Connect to the LatencyFleX service
	// LatencyFleX is an open source vendor agnostic replacement for Nvidia Reflex input latency reduction technology.
	// https://ishitatsuyuki.github.io/post/latencyflex/
	HMODULE pLfxModule;

	if (pLfxModule = LoadLibraryA("latencyflex_layer.dll"))
		m_winelfx_WaitAndBeginFrame = reinterpret_cast<void (*)()>(reinterpret_cast<void*>(GetProcAddress(pLfxModule, "lfx_WaitAndBeginFrame")));
	else if (pLfxModule = LoadLibraryA("latencyflex_wine.dll"))
		m_winelfx_WaitAndBeginFrame = reinterpret_cast<void (*)()>(reinterpret_cast<void*>(GetProcAddress(pLfxModule, "winelfx_WaitAndBeginFrame")));
	else
	{
		Warning(eLog::NS, "Unable to load LatencyFleX library, LatencyFleX disabled.\n");
		return;
	}

	o_OnRenderStart = module.Offset(0x1952C0).RCast<void (*)()>();
	HookAttach(&(PVOID&)o_OnRenderStart, (PVOID)h_OnRenderStart);

	DevMsg(eLog::NS, "LatencyFleX initialized.\n");
}
