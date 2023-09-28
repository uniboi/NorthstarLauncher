#include <Windows.h>

#include "primelauncher/launcher.h"

//-----------------------------------------------------------------------------
// Purpose: Entry point
//-----------------------------------------------------------------------------
BOOL WINAPI DllMain(HMODULE hModule, DWORD dwReason, LPVOID lpReserved)
{
	NOTE_UNUSED(hModule);
	NOTE_UNUSED(lpReserved);
	if (dwReason != DLL_PROCESS_ATTACH)
		return TRUE;

	g_pLauncher->InitCoreSubsystems();
	g_pLauncher->WaitForDebugger();

	if (!g_pLauncher->ShouldInject())
		return TRUE;

	g_pLauncher->InitNorthstarSubsystems();
	g_pLauncher->InjectNorthstar();

	return TRUE;
}
