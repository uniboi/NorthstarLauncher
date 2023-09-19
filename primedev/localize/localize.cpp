#include "mods/modmanager.h"

#include "gameui/GameConsole.h"
#include "vgui/vgui_baseui_interface.h"

class CLocalize;

CLocalize* g_pLocalize;

bool (*o_CLocalize__AddFile)(CLocalize* self, const char* path, const char* pathId, bool bIncludeFallbackSearchPaths);

bool h_CLocalize__AddFile(CLocalize* pVguiLocalize, const char* path, const char* pathId, bool bIncludeFallbackSearchPaths)
{
	bool ret = o_CLocalize__AddFile(pVguiLocalize, path, pathId, bIncludeFallbackSearchPaths);
	if (ret)
		DevMsg(eLog::MODSYS, "Loaded localisation file %s successfully\n", path);

	return true;
}

void (*o_CLocalize__ReloadLocalizationFiles)(CLocalize* self);

void h_CLocalize__ReloadLocalizationFiles(CLocalize* pVguiLocalize)
{
	// load all mod localization manually, so we keep track of all files, not just previously loaded ones
	for (Mod mod : g_pModManager->m_LoadedMods)
		if (mod.m_bEnabled)
			for (std::string& localisationFile : mod.LocalisationFiles)
				o_CLocalize__AddFile(g_pLocalize, localisationFile.c_str(), nullptr, false);

	DevMsg(eLog::MODSYS, "reloading localization...\n");
	o_CLocalize__ReloadLocalizationFiles(pVguiLocalize);
}

void (*o_CEngineVGui__Init)(void* self);

void h_CEngineVGui__Init(void* self)
{
	o_CEngineVGui__Init(self); // this loads r1_english, valve_english, dev_english

	g_bEngineVguiInitilased = true;

	g_pGameConsole->Initialize();

	// previously we did this in CLocalize::AddFile, but for some reason it won't properly overwrite localization from
	// files loaded previously if done there, very weird but this works so whatever
	for (Mod mod : g_pModManager->m_LoadedMods)
		if (mod.m_bEnabled)
			for (std::string& localisationFile : mod.LocalisationFiles)
				o_CLocalize__AddFile(g_pLocalize, localisationFile.c_str(), nullptr, false);
}

ON_DLL_LOAD_CLIENT("localize.dll", Localize, (CModule module))
{
	o_CLocalize__AddFile = module.Offset(0x6D80).RCast<bool (*)(CLocalize*, const char*, const char*, bool)>();
	HookAttach(&(PVOID&)o_CLocalize__AddFile, (PVOID)h_CLocalize__AddFile);

	o_CLocalize__ReloadLocalizationFiles = module.Offset(0xB830).RCast<void (*)(CLocalize*)>();
	HookAttach(&(PVOID&)o_CLocalize__ReloadLocalizationFiles, (PVOID)h_CLocalize__ReloadLocalizationFiles);

	g_pLocalize = Sys_GetFactoryPtr("localize.dll", "Localize_001").RCast<CLocalize*>();
}

ON_DLL_LOAD_CLIENT("engine.dll", LocalizeEngineVgui, (CModule module))
{
	o_CEngineVGui__Init = module.Offset(0x247E10).RCast<void (*)(void*)>();
	HookAttach(&(PVOID&)o_CEngineVGui__Init, (PVOID)h_CEngineVGui__Init);
}
