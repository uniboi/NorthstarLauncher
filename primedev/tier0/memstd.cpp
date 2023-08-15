#include "memstd.h"

//-----------------------------------------------------------------------------
// Purpose: Initilazes tier0 exports we use
//-----------------------------------------------------------------------------
static bool s_bMemStdInitilased = false;
static void MemStd_Init()
{
	if (!s_bMemStdInitilased)
	{
		HMODULE hTier0 = GetModuleHandleA("tier0.dll");
		//CModule mTier0(hTier0);

		//g_pMemAllocSingleton = reinterpret_cast<IMemAlloc*>(GetProcAddress(hTier0, "g_pMemAllocSingleton"));
		CreateGlobalMemAlloc = reinterpret_cast<IMemAlloc* (*)()>(GetProcAddress(hTier0, "CreateGlobalMemAlloc"));

		//g_pMemAllocSingleton = mTier0.GetExportedFunction("g_pMemAllocSingleton").Deref().RCast<IMemAlloc*>();
		//CreateGlobalMemAlloc = mTier0.GetExportedFunction("CreateGlobalMemAlloc").RCast<IMemAlloc* (*)()>();

		s_bMemStdInitilased = true;
	}
}

//-----------------------------------------------------------------------------
// Purpose: Returns g_pMemAllocSingleton
//-----------------------------------------------------------------------------
IMemAlloc* GlobalMemAllocSingleton()
{
	MemStd_Init();

	if (!g_pMemAllocSingleton)
	{
		g_pMemAllocSingleton = CreateGlobalMemAlloc();
	}

	return g_pMemAllocSingleton;
}
