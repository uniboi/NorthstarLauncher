#include "wconsole.h"

#include <Windows.h>
#include <cstdio>

#include "tier0/commandline.h"

//-----------------------------------------------------------------------------
// Purpose: Initilazes the windows console
//-----------------------------------------------------------------------------
void Console_Init(void)
{
	g_bConsole_UseAnsiColor = CommandLine()->CheckParm("-noansicolor") == nullptr;

	// Always show the console when starting-up.
	(void)AllocConsole();

	FILE* pDummy;
	freopen_s(&pDummy, "CONIN$", "r", stdin);
	freopen_s(&pDummy, "CONOUT$", "w", stdout);
	freopen_s(&pDummy, "CONOUT$", "w", stderr);

	if (g_bConsole_UseAnsiColor)
	{
		DWORD dwMode = 0;
		HANDLE hOutput = GetStdHandle(STD_OUTPUT_HANDLE);

		GetConsoleMode(hOutput, &dwMode);
		dwMode |= ENABLE_PROCESSED_OUTPUT | ENABLE_VIRTUAL_TERMINAL_PROCESSING;

		if (!SetConsoleMode(hOutput, dwMode))
		{
			g_bConsole_UseAnsiColor = false;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Closes the console window if wanted
//-----------------------------------------------------------------------------
void Console_PostInit(void)
{
	// Hide the console if user wants to
	bool bHide = CommandLine()->CheckParm("-wconsole") == nullptr && CommandLine()->CheckParm("-dedicated") == nullptr;

	if (!bHide)
		return;

	HWND hConsole = GetConsoleWindow();
	(void)FreeConsole();
	(void)PostMessageA(hConsole, WM_CLOSE, 0, 0);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
void Console_Shutdown(void) {}
