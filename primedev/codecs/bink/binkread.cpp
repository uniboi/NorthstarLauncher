#include "mods/modmanager.h"


void* (*o_BinkOpen)(const char* szPath, uint32_t iFlags);

void* h_BinkOpen(const char* path, uint32_t flags)
{
	std::string filename(fs::path(path).filename().string());
	DevMsg(eLog::VIDEO, "BinkOpen %s\n", filename.c_str());

	// figure out which mod is handling the bink
	Mod* fileOwner = nullptr;
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.m_bEnabled)
			continue;

		if (std::find(mod.BinkVideos.begin(), mod.BinkVideos.end(), filename) != mod.BinkVideos.end())
			fileOwner = &mod;
	}

	if (fileOwner)
	{
		// create new path
		fs::path binkPath(fileOwner->m_ModDirectory / "media" / filename);
		return o_BinkOpen(binkPath.string().c_str(), flags);
	}
	else
		return o_BinkOpen(path, flags);
}

ON_DLL_LOAD_CLIENT("bink2w64.dll", BinkRead, (CModule module))
{
	o_BinkOpen = module.GetExportedFunction("BinkOpen").RCast<void* (*)(const char*, uint32_t)>();
	HookAttach(&(PVOID&)o_BinkOpen, (PVOID)h_BinkOpen);
}
