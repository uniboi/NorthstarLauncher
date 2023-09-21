#include "game/server/ai_networkmanager.h"

#include "game/server/ai_helper.h"

void (*o_CAI_NetworkBuilder__Build)(void* self, CAI_Network* aiNetwork, void* unknown);

void h_CAI_NetworkBuilder__Build(void* builder, CAI_Network* aiNetwork, void* unknown)
{
	o_CAI_NetworkBuilder__Build(builder, aiNetwork, unknown);

	g_pAIHelper->SaveNetworkGraph(aiNetwork);
}

void (*o_CAI_NetworkManager__LoadNetworkGraph)(CAI_NetworkManager* self, void* buf, const char* filename);

void h_CAI_NetworkManager__LoadNetworkGraph(CAI_NetworkManager* self, void* buf, const char* filename)
{
	o_CAI_NetworkManager__LoadNetworkGraph(self, buf, filename);

	if (Cvar_ns_ai_dumpAINfileFromLoad->GetBool())
	{
		DevMsg(eLog::NS, "running DumpAINInfo for loaded file %s\n", filename);

		g_pAIHelper->SaveNetworkGraph(self->m_pNetwork);
	}
}

ON_DLL_LOAD("server.dll", AINetworkManager, (CModule module))
{
	o_CAI_NetworkBuilder__Build = module.Offset(0x385E20).RCast<void (*)(void*, CAI_Network*, void*)>();
	HookAttach(&(PVOID&)o_CAI_NetworkBuilder__Build, (PVOID)h_CAI_NetworkBuilder__Build);

	o_CAI_NetworkManager__LoadNetworkGraph = module.Offset(0x3933A0).RCast<void (*)(CAI_NetworkManager*, void*, const char*)>();
	HookAttach(&(PVOID&)o_CAI_NetworkManager__LoadNetworkGraph, (PVOID)h_CAI_NetworkManager__LoadNetworkGraph);

	g_pAINetwork = module.Offset(0x1061160).RCast<CAI_Network**>();
	g_pNetworkManager = module.Offset(0x10613F8).RCast<CAI_NetworkManager**>();
}
