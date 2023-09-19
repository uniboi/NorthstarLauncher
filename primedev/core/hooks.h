#pragma once

#include <string>
#include <iostream>

//-----------------------------------------------------------------------------
// Purpose: MH_MakeHook wrapper
// Input  : *ppOriginal - Original function being detoured
//          pDetour - Detour function
//-----------------------------------------------------------------------------
inline void HookAttach(PVOID* ppOriginal, PVOID pDetour)
{
	PVOID pAddr = *ppOriginal;
	if (MH_CreateHook(pAddr, pDetour, ppOriginal) == MH_OK)
	{
		if (MH_EnableHook(pAddr) != MH_OK)
		{
			Error(eLog::NS, EXIT_FAILURE, "Failed enabling a function hook!\n");
		}
	}
	else
	{
		Error(eLog::NS, EXIT_FAILURE, "Failed creating a function hook!\n");
	}
}

void CallLoadLibraryACallbacks(LPCSTR lpLibFileName, HMODULE moduleAddress);

typedef void (*DllLoadCallbackFuncType)(CModule moduleAddress);
void AddDllLoadCallback(std::string dll, DllLoadCallbackFuncType callback, std::string tag = "", std::vector<std::string> reliesOn = {});
void AddDllLoadCallbackForDedicatedServer(std::string dll, DllLoadCallbackFuncType callback, std::string tag = "", std::vector<std::string> reliesOn = {});
void AddDllLoadCallbackForClient(std::string dll, DllLoadCallbackFuncType callback, std::string tag = "", std::vector<std::string> reliesOn = {});

void CallAllPendingDLLLoadCallbacks();

// new dll load callback stuff
enum class eDllLoadCallbackSide
{
	UNSIDED,
	CLIENT,
	DEDICATED_SERVER
};

class __dllLoadCallback
{
  public:
	__dllLoadCallback() = delete;
	__dllLoadCallback(eDllLoadCallbackSide side, const std::string dllName, DllLoadCallbackFuncType callback, std::string uniqueStr, std::string reliesOn);
};

#define __CONCAT3(x, y, z) x##y##z
#define CONCAT3(x, y, z) __CONCAT3(x, y, z)
#define __CONCAT2(x, y) x##y
#define CONCAT2(x, y) __CONCAT2(x, y)
#define __STR(s) #s

// adds a callback to be called when a given dll is loaded, for creating hooks and such
#define __ON_DLL_LOAD(dllName, side, uniquestr, reliesOn, args)                                                                                           \
	void CONCAT2(__dllLoadCallback, uniquestr) args;                                                                                                      \
	namespace                                                                                                                                             \
	{                                                                                                                                                     \
		__dllLoadCallback CONCAT2(__dllLoadCallbackInstance, __LINE__)(side, dllName, CONCAT2(__dllLoadCallback, uniquestr), __STR(uniquestr), reliesOn); \
	}                                                                                                                                                     \
	void CONCAT2(__dllLoadCallback, uniquestr) args

#define ON_DLL_LOAD(dllName, uniquestr, args) __ON_DLL_LOAD(dllName, eDllLoadCallbackSide::UNSIDED, uniquestr, "", args)
#define ON_DLL_LOAD_RELIESON(dllName, uniquestr, reliesOn, args) __ON_DLL_LOAD(dllName, eDllLoadCallbackSide::UNSIDED, uniquestr, __STR(reliesOn), args)
#define ON_DLL_LOAD_CLIENT(dllName, uniquestr, args) __ON_DLL_LOAD(dllName, eDllLoadCallbackSide::CLIENT, uniquestr, "", args)
#define ON_DLL_LOAD_CLIENT_RELIESON(dllName, uniquestr, reliesOn, args) __ON_DLL_LOAD(dllName, eDllLoadCallbackSide::CLIENT, uniquestr, __STR(reliesOn), args)
#define ON_DLL_LOAD_DEDI(dllName, uniquestr, args) __ON_DLL_LOAD(dllName, eDllLoadCallbackSide::DEDICATED_SERVER, uniquestr, "", args)
#define ON_DLL_LOAD_DEDI_RELIESON(dllName, uniquestr, reliesOn, args) __ON_DLL_LOAD(dllName, eDllLoadCallbackSide::DEDICATED_SERVER, uniquestr, __STR(reliesOn), args)

void MakeHook(LPVOID pTarget, LPVOID pDetour, void* ppOriginal, const char* pFuncName = "");
#define MAKEHOOK(pTarget, pDetour, ppOriginal) MakeHook((LPVOID)pTarget, (LPVOID)pDetour, (void*)ppOriginal, __STR(pDetour))
