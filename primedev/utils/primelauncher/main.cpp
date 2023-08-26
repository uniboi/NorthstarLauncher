#include "Windows.h"
#include "launcher.h"

extern "C"
{
	__declspec(dllexport) DWORD AmdPowerXpressRequestHighPerformance = 0x00000001;
	__declspec(dllexport) DWORD NvOptimusEnablement = 0x00000001;
}

//-----------------------------------------------------------------------------
// Purpose: Entry point
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	g_pLauncher->InitCoreSubsystems();
	g_pLauncher->WaitForDebugger();

	g_pLauncher->InitOrigin();

	if (!g_pLauncher->ShouldInject())
		return 0;

	g_pLauncher->InitNorthstarSubsystems();
	g_pLauncher->InjectNorthstar();

	return 0;
}
