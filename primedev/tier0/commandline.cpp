
#include "tier0/commandline.h"

void (*o_CCommandLine_CreateCmdLine)(CCommandLine* self, const char* pszCommandLine);

void h_CCommandLine_CreateCmdLine(CCommandLine* self, const char* pszCommandLine)
{
	NOTE_UNUSED(pszCommandLine);

	if (!self->ParmCount())
		Error(eLog::NS, EXIT_FAILURE, "Commandline not created in LauncherMain\n");

	// Ensure we dont try to create cmd multiple times
	return;
}

ON_DLL_LOAD("tier0.dll", Tier0CommandLine, (CModule module))
{
	o_CCommandLine_CreateCmdLine = module.Offset(0xBBB0).RCast<void (*)(CCommandLine*, const char*)>();
	HookAttach(&(PVOID&)o_CCommandLine_CreateCmdLine, (PVOID)h_CCommandLine_CreateCmdLine);
}
