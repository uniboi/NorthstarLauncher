#include "dedicated/dedicated.h"

#include <iostream>
#include <wchar.h>
#include <iostream>
#include <vector>
#include <fstream>
#include <sstream>
#include <filesystem>
#include <Psapi.h>

// called from the ON_DLL_LOAD macros
__dllLoadCallback::__dllLoadCallback(eDllLoadCallbackSide side, const std::string dllName, DllLoadCallbackFuncType callback, std::string uniqueStr, std::string reliesOn)
{
	// parse reliesOn array from string
	std::vector<std::string> reliesOnArray;

	if (reliesOn.length() && reliesOn[0] != '(')
	{
		reliesOnArray.push_back(reliesOn);
	}
	else
	{
		// follows the format (tag, tag, tag)
		std::string sCurrentTag;
		for (int i = 1; i < reliesOn.length(); i++)
		{
			if (!isspace(reliesOn[i]))
			{
				if (reliesOn[i] == ',' || reliesOn[i] == ')')
				{
					reliesOnArray.push_back(sCurrentTag);
					sCurrentTag = "";
				}
				else
					sCurrentTag += reliesOn[i];
			}
		}
	}

	switch (side)
	{
	case eDllLoadCallbackSide::UNSIDED:
	{
		AddDllLoadCallback(dllName, callback, uniqueStr, reliesOnArray);
		break;
	}

	case eDllLoadCallbackSide::CLIENT:
	{
		AddDllLoadCallbackForClient(dllName, callback, uniqueStr, reliesOnArray);
		break;
	}

	case eDllLoadCallbackSide::DEDICATED_SERVER:
	{
		AddDllLoadCallbackForDedicatedServer(dllName, callback, uniqueStr, reliesOnArray);
		break;
	}
	}
}

// dll load callback stuff
// this allows for code to register callbacks to be run as soon as a dll is loaded, mainly to allow for patches to be made on dll load
struct DllLoadCallback
{
	std::string dll;
	DllLoadCallbackFuncType callback;
	std::string tag;
	std::vector<std::string> reliesOn;
	bool called;
};

// HACK: declaring and initialising this vector at file scope crashes on debug builds due to static initialisation order
// using a static var like this ensures that the vector is initialised lazily when it's used
std::vector<DllLoadCallback>& GetDllLoadCallbacks()
{
	static std::vector<DllLoadCallback> vec = std::vector<DllLoadCallback>();
	return vec;
}

void AddDllLoadCallback(std::string dll, DllLoadCallbackFuncType callback, std::string tag, std::vector<std::string> reliesOn)
{
	DllLoadCallback& callbackStruct = GetDllLoadCallbacks().emplace_back();

	callbackStruct.dll = dll;
	callbackStruct.callback = callback;
	callbackStruct.tag = tag;
	callbackStruct.reliesOn = reliesOn;
	callbackStruct.called = false;
}

void AddDllLoadCallbackForDedicatedServer(std::string dll, DllLoadCallbackFuncType callback, std::string tag, std::vector<std::string> reliesOn)
{
	if (!IsDedicatedServer())
		return;

	AddDllLoadCallback(dll, callback, tag, reliesOn);
}

void AddDllLoadCallbackForClient(std::string dll, DllLoadCallbackFuncType callback, std::string tag, std::vector<std::string> reliesOn)
{
	if (IsDedicatedServer())
		return;

	AddDllLoadCallback(dll, callback, tag, reliesOn);
}

void MakeHook(LPVOID pTarget, LPVOID pDetour, void* ppOriginal, const char* pFuncName)
{
	char* pStrippedFuncName = (char*)pFuncName;
	// strip & char from funcname
	if (*pStrippedFuncName == '&')
		pStrippedFuncName++;

	if (MH_CreateHook(pTarget, pDetour, (LPVOID*)ppOriginal) == MH_OK)
	{
		if (MH_EnableHook(pTarget) == MH_OK)
			DevMsg(eLog::NS, "Enabling hook %s\n", pStrippedFuncName);
		else
			Error(eLog::NS, NO_ERROR, "MH_EnableHook failed for function %s\n", pStrippedFuncName);
	}
	else
		Error(eLog::NS, NO_ERROR, "MH_CreateHook failed for function %s\n", pStrippedFuncName);
}

std::vector<std::string> calledTags;
void CallLoadLibraryACallbacks(LPCSTR lpLibFileName, HMODULE moduleAddress)
{
	CModule cModule(moduleAddress);

	while (true)
	{
		bool bDoneCalling = true;

		for (auto& callbackStruct : GetDllLoadCallbacks())
		{
			if (!callbackStruct.called && fs::path(lpLibFileName).filename() == fs::path(callbackStruct.dll).filename())
			{
				bool bShouldContinue = false;

				if (!callbackStruct.reliesOn.empty())
				{
					for (std::string tag : callbackStruct.reliesOn)
					{
						if (std::find(calledTags.begin(), calledTags.end(), tag) == calledTags.end())
						{
							bDoneCalling = false;
							bShouldContinue = true;
							break;
						}
					}
				}

				if (bShouldContinue)
					continue;

				callbackStruct.callback(moduleAddress);
				calledTags.push_back(callbackStruct.tag);
				callbackStruct.called = true;
			}
		}

		if (bDoneCalling)
			break;
	}
}

void CallLoadLibraryWCallbacks(LPCWSTR lpLibFileName, HMODULE moduleAddress)
{
	CModule cModule(moduleAddress);

	while (true)
	{
		bool bDoneCalling = true;

		for (auto& callbackStruct : GetDllLoadCallbacks())
		{
			if (!callbackStruct.called && fs::path(lpLibFileName).filename() == fs::path(callbackStruct.dll).filename())
			{
				bool bShouldContinue = false;

				if (!callbackStruct.reliesOn.empty())
				{
					for (std::string tag : callbackStruct.reliesOn)
					{
						if (std::find(calledTags.begin(), calledTags.end(), tag) == calledTags.end())
						{
							bDoneCalling = false;
							bShouldContinue = true;
							break;
						}
					}
				}

				if (bShouldContinue)
					continue;

				callbackStruct.callback(moduleAddress);
				calledTags.push_back(callbackStruct.tag);
				callbackStruct.called = true;
			}
		}

		if (bDoneCalling)
			break;
	}
}

void CallAllPendingDLLLoadCallbacks()
{
	HMODULE hMods[1024];
	HANDLE hProcess = GetCurrentProcess();
	DWORD cbNeeded;
	unsigned int i;

	// Get a list of all the modules in this process.
	if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded))
	{
		for (i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			wchar_t szModName[MAX_PATH];

			// Get the full path to the module's file.
			if (GetModuleFileNameExW(hProcess, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				CallLoadLibraryWCallbacks(szModName, hMods[i]);
			}
		}
	}
}
