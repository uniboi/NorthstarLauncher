#include <Windows.h>

#include "tier0/filestream.h"
#include <signal.h>

std::string svPrefix;
std::string svException;
std::string svModule;
std::string svOffset;

//-----------------------------------------------------------------------------
// Purpose: Gets argument at index
// Input  : nIdx -
//          argc - 
//          *argv[] -
// Output : Zero terminated string
//-----------------------------------------------------------------------------
const char* GetArg(int nIdx, int argc, char* argv[])
{
	if (nIdx >= argc)
		return "INVALID_INDEX";

	return argv[nIdx];
}

//-----------------------------------------------------------------------------
// Purpose: Gets the remaining arguments, valid paths can have spaces so we
//          put the profile dir as teh last arg
// Input  : nIdx -
//          argc -
//          *argv[] -
// Output : string
//-----------------------------------------------------------------------------
std::string GetRemainingArgs(int nIdx, int argc, char* argv[])
{
	if (nIdx >= argc)
		return "INVALID_INDEX";

	std::string svString;
	for (int i = nIdx; i < argc; i++)
	{
		svString += argv[i];

		if (i + 1 != argc)
			svString += " ";
	}

	return svString;
}

//-----------------------------------------------------------------------------
// Purpose: Entry point
//-----------------------------------------------------------------------------
int main(int argc, char* argv[])
{
	svException = GetArg(1, argc, argv);
	svModule = GetArg(2, argc, argv);
	svOffset = GetArg(3, argc, argv);
	svPrefix = GetRemainingArgs(4, argc, argv);

	std::string svMessage = FormatA("Crash info can be found at %s/logs!\n\n%s\n%s + %s\n\nPress \"Ctrl + C\" to copy this message.", svPrefix.c_str(), svException.c_str(), svModule.c_str(), svOffset.c_str());

	MessageBoxA(GetActiveWindow(), svMessage.c_str(), "Northstar Prime has crashed!", MB_ICONERROR | MB_OK);

	return 0;
}
