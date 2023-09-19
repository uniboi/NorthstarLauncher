#include "filesystem/basefilesystem.h"
#include "mods/modmanager.h"

#include <iostream>
#include <sstream>

bool bReadingOriginalFile = false;
std::string sCurrentModPath;

IFileSystem* g_pFilesystem;

std::string ReadVPKFile(const char* path)
{
	// read scripts.rson file, todo: check if this can be overwritten
	FileHandle_t fileHandle = g_pFilesystem->m_vtable2->Open(&g_pFilesystem->m_vtable2, path, "rb", "GAME", 0);

	std::stringstream fileStream;
	int bytesRead = 0;
	char data[4096];
	do
	{
		bytesRead = g_pFilesystem->m_vtable2->Read(&g_pFilesystem->m_vtable2, data, (int)std::size(data), fileHandle);
		fileStream.write(data, bytesRead);
	} while (bytesRead == std::size(data));

	g_pFilesystem->m_vtable2->Close(g_pFilesystem, fileHandle);

	return fileStream.str();
}

std::string ReadVPKOriginalFile(const char* path)
{
	// todo: should probably set search path to be g_pEngineParms->szModName here also

	bReadingOriginalFile = true;
	std::string ret = ReadVPKFile(path);
	bReadingOriginalFile = false;

	return ret;
}

void (*o_CBaseFileSystem__AddSearchPath)(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType);

void h_CBaseFileSystem__AddSearchPath(IFileSystem* fileSystem, const char* pPath, const char* pathID, SearchPathAdd_t addType)
{
	o_CBaseFileSystem__AddSearchPath(fileSystem, pPath, pathID, addType);

	// make sure current mod paths are at head
	if (!strcmp(pathID, "GAME") && sCurrentModPath.compare(pPath) && addType == PATH_ADD_TO_HEAD)
	{
		o_CBaseFileSystem__AddSearchPath(fileSystem, sCurrentModPath.c_str(), "GAME", PATH_ADD_TO_HEAD);
		o_CBaseFileSystem__AddSearchPath(fileSystem, GetCompiledAssetsPath().string().c_str(), "GAME", PATH_ADD_TO_HEAD);
	}
}

void SetNewModSearchPaths(Mod* mod)
{
	// put our new path to the head if we need to read from a different mod path
	// in the future we could also determine whether the file we're setting paths for needs a mod dir, or compiled assets
	if (mod != nullptr)
	{
		if ((fs::absolute(mod->m_ModDirectory) / MOD_OVERRIDE_DIR).string().compare(sCurrentModPath))
		{
			// NOTE [Fifty]: Possibly put this behind some check
			// DevMsg(eLog::FS, "Changing mod search path from %s to %s\n", sCurrentModPath.c_str(), mod->m_ModDirectory.string().c_str());

			o_CBaseFileSystem__AddSearchPath(&*g_pFilesystem, (fs::absolute(mod->m_ModDirectory) / MOD_OVERRIDE_DIR).string().c_str(), "GAME", PATH_ADD_TO_HEAD);
			sCurrentModPath = (fs::absolute(mod->m_ModDirectory) / MOD_OVERRIDE_DIR).string();
		}
	}
	else // push compiled to head
		o_CBaseFileSystem__AddSearchPath(&*g_pFilesystem, fs::absolute(GetCompiledAssetsPath()).string().c_str(), "GAME", PATH_ADD_TO_HEAD);
}

bool TryReplaceFile(const char* pPath, bool shouldCompile)
{
	if (bReadingOriginalFile)
		return false;

	if (shouldCompile)
		g_pModManager->CompileAssetsForFile(pPath);

	// idk how efficient the lexically normal check is
	// can't just set all /s in path to \, since some paths aren't in writeable memory
	auto file = g_pModManager->m_ModFiles.find(g_pModManager->NormaliseModFilePath(fs::path(pPath)));
	if (file != g_pModManager->m_ModFiles.end())
	{
		SetNewModSearchPaths(file->second.m_pOwningMod);
		return true;
	}

	return false;
}

// force modded files to be read from mods, not cache
bool (*o_CBaseFileSystem__ReadFromCache)(IFileSystem* filesystem, char* pPath, void* result);

bool h_CBaseFileSystem__ReadFromCache(IFileSystem* filesystem, char* pPath, void* result)
{
	if (TryReplaceFile(pPath, true))
		return false;

	return o_CBaseFileSystem__ReadFromCache(filesystem, pPath, result);
}

// force modded files to be read from mods, not vpk
FileHandle_t (*o_CBaseFileSystem__ReadFileFromVPK)(VPKData* vpkInfo, uint64_t* b, char* filename);

FileHandle_t h_CBaseFileSystem__ReadFileFromVPK(VPKData* vpkInfo, uint64_t* b, char* filename)
{
	// don't compile here because this is only ever called from OpenEx, which already compiles
	if (TryReplaceFile(filename, false))
	{
		*b = -1;
		return b;
	}

	return o_CBaseFileSystem__ReadFileFromVPK(vpkInfo, b, filename);
}

FileHandle_t (*o_CBaseFileSystem__OpenEx)(IFileSystem* filesystem, const char* pPath, const char* pOptions, uint32_t flags, const char* pPathID, char** ppszResolvedFilename);

FileHandle_t h_CBaseFileSystem__OpenEx(IFileSystem* filesystem, const char* pPath, const char* pOptions, uint32_t flags, const char* pPathID, char** ppszResolvedFilename)
{
	TryReplaceFile(pPath, true);
	return o_CBaseFileSystem__OpenEx(filesystem, pPath, pOptions, flags, pPathID, ppszResolvedFilename);
}

VPKData* (*o_CBaseFileSystem__MountVPK)(IFileSystem* fileSystem, const char* pVpkPath);

VPKData* h_CBaseFileSystem__MountVPK(IFileSystem * fileSystem, const char* pVpkPath)
{
	DevMsg(eLog::FS, "MountVPK %s\n", pVpkPath);
	VPKData* ret = o_CBaseFileSystem__MountVPK(fileSystem, pVpkPath);

	for (Mod mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.m_bEnabled)
			continue;

		for (ModVPKEntry& vpkEntry : mod.Vpks)
		{
			// if we're autoloading, just load no matter what
			if (!vpkEntry.m_bAutoLoad)
			{
				// resolve vpk name and try to load one with the same name
				// todo: we should be unloading these on map unload manually
				std::string mapName(fs::path(pVpkPath).filename().string());
				std::string modMapName(fs::path(vpkEntry.m_sVpkPath.c_str()).filename().string());
				if (mapName.compare(modMapName))
					continue;
			}

			VPKData* loaded = o_CBaseFileSystem__MountVPK(fileSystem, vpkEntry.m_sVpkPath.c_str());
			if (!ret) // this is primarily for map vpks and stuff, so the map's vpk is what gets returned from here
				ret = loaded;
		}
	}

	return ret;
}

ON_DLL_LOAD("filesystem_stdio.dll", Filesystem, (CModule module))
{
	o_CBaseFileSystem__AddSearchPath = module.Offset(0xb510).RCast<void (*)(IFileSystem*, const char*, const char*, SearchPathAdd_t)>();
	HookAttach(&(PVOID&)o_CBaseFileSystem__AddSearchPath, (PVOID)h_CBaseFileSystem__AddSearchPath);

	o_CBaseFileSystem__ReadFromCache = module.Offset(0xfe50).RCast<bool (*)(IFileSystem*, char*, void*)>();
	HookAttach(&(PVOID&)o_CBaseFileSystem__ReadFromCache, (PVOID)h_CBaseFileSystem__ReadFromCache);

	o_CBaseFileSystem__ReadFileFromVPK = module.Offset(0x5CBA0).RCast<FileHandle_t (*)(VPKData*, uint64_t*, char*)>();
	HookAttach(&(PVOID&)o_CBaseFileSystem__ReadFileFromVPK, (PVOID)h_CBaseFileSystem__ReadFileFromVPK);

	o_CBaseFileSystem__OpenEx = module.Offset(0x15F50).RCast<FileHandle_t (*)(IFileSystem*, const char*, const char*, uint32_t, const char*, char**)>();
	HookAttach(&(PVOID&)o_CBaseFileSystem__OpenEx, (PVOID)h_CBaseFileSystem__OpenEx);

	o_CBaseFileSystem__MountVPK = module.Offset(0xbea0).RCast<VPKData* (*)(IFileSystem*, const char*)>();
	HookAttach(&(PVOID&)o_CBaseFileSystem__MountVPK, (PVOID)h_CBaseFileSystem__MountVPK);

	g_pFilesystem = Sys_GetFactoryPtr("filesystem_stdio.dll", "VFileSystem017").RCast<IFileSystem*>();
}
