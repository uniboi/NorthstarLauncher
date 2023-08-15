#include "mods/modmanager.h"

AUTOHOOK_INIT()

// clang-format off
AUTOHOOK_PROCADDRESS(BinkOpen, bink2w64.dll, BinkOpen, 
void*, __fastcall, (const char* path, uint32_t flags))
// clang-format on
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
		return BinkOpen(binkPath.string().c_str(), flags);
	}
	else
		return BinkOpen(path, flags);
}

ON_DLL_LOAD_CLIENT("bink2w64.dll", BinkRead, (CModule module))
{
	AUTOHOOK_DISPATCH()
}
