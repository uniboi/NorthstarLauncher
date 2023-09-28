#include "modmanager.h"
#include "tier1/convar.h"
#include "tier1/cmd.h"
#include "mods/audio.h"
#include "filesystem/basefilesystem.h"
#include "rtech/pakapi.h"
#include "vscript/vscript.h"

#include <regex>

#include "tier0/filestream.h"
#include "tier0/taskscheduler.h"

ModManager* g_pModManager;

Mod::Mod(fs::path modDir, std::string& svModJson)
{
	m_bWasReadSuccessfully = false;

	m_ModDirectory = modDir;

	try
	{
		nlohmann::json jsMod = nlohmann::json::parse(svModJson);

		// load name first
		Name = jsMod["Name"].get<std::string>();

		// Don't load blacklisted mods
		if (!strstr(GetCommandLineA(), "-nomodblacklist") && MODS_BLACKLIST.find(Name) != std::end(MODS_BLACKLIST))
		{
			Warning(eLog::MODSYS, "Skipping blacklisted mod \"%s\"!\n", Name.c_str());
			return;
		}

		Description = jsMod["Description"].get<std::string>();
		Version = jsMod["Version"].get<std::string>();

		if (jsMod.contains("DownloadLink"))
			DownloadLink = jsMod["DownloadLink"].get<std::string>();
		else
			DownloadLink = "";

		if (jsMod.contains("RequiredOnClient"))
			RequiredOnClient = jsMod["RequiredOnClient"].get<bool>();
		else
			RequiredOnClient = false;

		LoadPriority = jsMod["LoadPriority"].get<int>();

		// Parse Convars
		if (jsMod.contains("ConVars"))
		{
			for (auto& jsConVar : jsMod["ConVars"])
			{
				std::string svName = jsConVar["Name"].get<std::string>();
				std::string svDefaultValue = jsConVar["DefaultValue"].get<std::string>();

				std::string svHelpString = "";
				if (jsConVar.contains("HelpString"))
					svHelpString = jsConVar["HelpString"].get<std::string>();

				int nFlags = FCVAR_NONE;
				if (jsConVar.contains("Flags"))
				{
					if (jsConVar["Flags"].is_number_integer())
						nFlags = jsConVar["Flags"].get<int>();
					else if (jsConVar["Flags"].is_string())
						nFlags = ParseConVarFlagsString(svName, jsConVar["Flags"].get<std::string>());
					else
						throw std::exception("custom_parse_error: ConVar::Flags has invalid type");
				}

				// Has to be manually allocated as convar registarion breaks otherwise
				// FIXME [Fifty]: We DONT deallocate this, this leaks memory and is bad
				ModConVar* pConVar = new ModConVar;
				pConVar->Name = svName;
				pConVar->HelpString = svHelpString;
				pConVar->DefaultValue = svDefaultValue;
				pConVar->Flags = nFlags;

				ConVars.emplace_back(pConVar);
			}
		}

		// Parse Concommands
		if (jsMod.contains("ConCommands"))
		{
			for (auto& jsConCommand : jsMod["ConCommands"])
			{
				std::string svName = jsConCommand["Name"].get<std::string>();
				std::string svFunction = jsConCommand["Function"].get<std::string>();
				std::string svContext = jsConCommand["Context"].get<std::string>();

				ScriptContext eContext = VScript_GetContextFromString(svContext);
				if (eContext == ScriptContext::INVALID)
					throw std::exception("custom_parse_error: ConCommand::Context is invalid");

				std::string svHelpString = "";
				if (jsConCommand.contains("HelpString"))
					svHelpString = jsConCommand["HelpString"].get<std::string>();

				int nFlags = FCVAR_NONE;
				if (jsConCommand.contains("Flags"))
				{
					if (jsConCommand["Flags"].is_number_integer())
						nFlags = jsConCommand["Flags"].get<int>();
					else if (jsConCommand["Flags"].is_string())
						nFlags = ParseConVarFlagsString(svName, jsConCommand["Flags"].get<std::string>());
					else
						throw std::exception("custom_parse_error: ConCommand::Flags has invalid type");
				}

				// Has to be manually allocated as convar registarion breaks otherwise
				// FIXME [Fifty]: We DONT deallocate this, this leaks memory and is bad
				ModConCommand* pConCommand = new ModConCommand;
				pConCommand->Name = svName;
				pConCommand->Function = svFunction;
				pConCommand->Context = eContext;
				pConCommand->HelpString = svHelpString;
				pConCommand->Flags = nFlags;

				ConCommands.emplace_back(pConCommand);
			}
		}

		// Parse Scripts
		if (jsMod.contains("Scripts"))
		{
			for (auto& jsScript : jsMod["Scripts"])
			{
				ModScript script;

				script.Path = jsScript["Path"].get<std::string>();
				script.RunOn = jsScript["RunOn"].get<std::string>();

				if (jsScript.contains("ServerCallback"))
				{
					ModScriptCallback callback;
					callback.Context = ScriptContext::SERVER;

					if (jsScript["ServerCallback"].contains("Before"))
						callback.BeforeCallback = jsScript["ServerCallback"]["Before"].get<std::string>();

					if (jsScript["ServerCallback"].contains("After"))
						callback.AfterCallback = jsScript["ServerCallback"]["After"].get<std::string>();

					if (jsScript["ServerCallback"].contains("Destroy"))
						callback.DestroyCallback = jsScript["ServerCallback"]["Destroy"].get<std::string>();

					script.Callbacks.emplace_back(callback);
				}

				if (jsScript.contains("ClientCallback"))
				{
					ModScriptCallback callback;
					callback.Context = ScriptContext::CLIENT;

					if (jsScript["ClientCallback"].contains("Before"))
						callback.BeforeCallback = jsScript["ClientCallback"]["Before"].get<std::string>();

					if (jsScript["ClientCallback"].contains("After"))
						callback.AfterCallback = jsScript["ClientCallback"]["After"].get<std::string>();

					if (jsScript["ClientCallback"].contains("Destroy"))
						callback.DestroyCallback = jsScript["ClientCallback"]["Destroy"].get<std::string>();

					script.Callbacks.emplace_back(callback);
				}

				if (jsScript.contains("UICallback"))
				{
					ModScriptCallback callback;
					callback.Context = ScriptContext::UI;

					if (jsScript["UICallback"].contains("Before"))
						callback.BeforeCallback = jsScript["UICallback"]["Before"].get<std::string>();

					if (jsScript["UICallback"].contains("After"))
						callback.AfterCallback = jsScript["UICallback"]["After"].get<std::string>();

					if (jsScript["UICallback"].contains("Destroy"))
						callback.DestroyCallback = jsScript["UICallback"]["Destroy"].get<std::string>();

					script.Callbacks.emplace_back(callback);
				}

				Scripts.emplace_back(script);
			}
		}

		// Parse Localization
		if (jsMod.contains("Localisation"))
		{
			for (auto& jsLoc : jsMod["Localisation"])
			{
				LocalisationFiles.emplace_back(jsLoc.get<std::string>());
			}
		}

		// Parse Dependencies
		if (jsMod.contains("Dependencies"))
		{
			for (auto& jsDep : jsMod["Dependencies"].items())
			{
				std::string svKey = jsDep.key();
				std::string svValue = jsDep.value();

				if (DependencyConstants.find(svKey) != DependencyConstants.end())
					throw std::exception("custom_parse_error: Duplicate dependency constant!");

				DependencyConstants.emplace(svKey, svValue);
			}
		}

		// Parse InitScript
		if (jsMod.contains("InitScript"))
		{
			initScript = jsMod["InitScript"].get<std::string>();
		}
	}
	catch (const std::exception& ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "Failed to parse mod.json in '%s': %s\n", modDir.string().c_str(), ex.what());
		return;
	}

	m_bWasReadSuccessfully = true;
	DevMsg(eLog::MODSYS, "Succesfully parsed mod: '%s'!\n", Name.c_str());
}

ModManager::ModManager()
{
	// precaculated string hashes
	// note: use backslashes for these, since we use lexically_normal for file paths which uses them
	m_hScriptsRsonHash = STR_HASH("scripts\\vscripts\\scripts.rson");
	m_hPdefHash = STR_HASH("cfg\\server\\persistent_player_data_version_231.pdef" // this can have multiple versions, but we use 231 so that's what we hash
	);
	m_hKBActHash = STR_HASH("scripts\\kb_act.lst");

	LoadMods();
}

template <ScriptContext context>
void ModConCommandCallback_Internal(std::string name, const CCommand& command)
{
	CSquirrelVM* pVM = nullptr;
	if constexpr(context == ScriptContext::SERVER)
		pVM = g_pServerVM;
	else if constexpr (context == ScriptContext::CLIENT)
		pVM = g_pClientVM;
	else if constexpr (context == ScriptContext::UI)
		pVM = g_pUIVM;

	if (pVM && pVM->GetVM())
	{
		// FIXME [Fifty]: 'command.ArgC()' is user dependent, we've basically give the user the choice to script error on command
		if (command.ArgC() == 1)
		{
			g_pTaskScheduler->AddTask(
				[name]()
				{
					CSquirrelVM* pVM = nullptr;
					if constexpr (context == ScriptContext::SERVER)
						pVM = g_pServerVM;
					else if constexpr (context == ScriptContext::CLIENT)
						pVM = g_pClientVM;
					else if constexpr (context == ScriptContext::UI)
						pVM = g_pUIVM;

					if (pVM && pVM->GetVM())
					{
						ScriptContext nContext = (ScriptContext)pVM->vmContext;
						HSQUIRRELVM hVM = pVM->GetVM();

						SQObject oFunction {};
						int nResult = sq_getfunction(hVM, name.c_str(), &oFunction, 0);
						if (nResult != 0)
						{
							Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", name.c_str());
							return;
						}

						// Push
						sq_pushobject(hVM, &oFunction);
						sq_pushroottable(hVM);

						(void)sq_call(hVM, 1, false, false);
					}
				});
		}
		else
		{
			std::vector<std::string> vArgs;
			for (int i = 1; i < command.ArgC(); i++)
				vArgs.push_back(command.Arg(i));

			g_pTaskScheduler->AddTask(
				[name, vArgs]()
				{
					CSquirrelVM* pVM = nullptr;
					if constexpr (context == ScriptContext::SERVER)
						pVM = g_pServerVM;
					else if constexpr (context == ScriptContext::CLIENT)
						pVM = g_pClientVM;
					else if constexpr(context == ScriptContext::UI)
						pVM = g_pUIVM;

					if (pVM && pVM->GetVM())
					{
						ScriptContext nContext = (ScriptContext)pVM->vmContext;
						HSQUIRRELVM hVM = pVM->GetVM();

						SQObject oFunction {};
						int nResult = sq_getfunction(hVM, name.c_str(), &oFunction, 0);
						if (nResult != 0)
						{
							Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", name.c_str());
							return;
						}

						// Push
						sq_pushobject(hVM, &oFunction);
						sq_pushroottable(hVM);

						sq_newarray(hVM, 0);

						for (int i = 0; i < vArgs.size(); i++)
						{
							sq_pushstring(hVM, vArgs[i].c_str(), -1);
							sq_arrayappend(hVM, -2);
						}

						(void)sq_call(hVM, vArgs.size() + 1, false, false);
					}
				});
		}
	}
	else
	{
		Warning(eLog::MODSYS, "ConCommand `%s` was called while the associated Squirrel VM `%s` was unloaded\n", name.c_str(), VScript_GetContextAsString(context));
	}
}

void ModConCommandCallback(const CCommand& command)
{
	ModConCommand* found = nullptr;
	std::string commandString = std::string(command.GetCommandString());

	// Finding the first space to remove the command's name
	auto firstSpace = commandString.find(' ');
	if (firstSpace)
	{
		commandString = commandString.substr(0, firstSpace);
	}

	// Find the mod this command belongs to
	for (auto& mod : g_pModManager->m_LoadedMods)
	{
		auto res = std::find_if(mod.ConCommands.begin(), mod.ConCommands.end(), [&commandString](const ModConCommand* other) { return other->Name == commandString; });
		if (res != mod.ConCommands.end())
		{
			found = *res;
			break;
		}
	}
	if (!found)
		return;

	switch (found->Context)
	{
	case ScriptContext::CLIENT:
		ModConCommandCallback_Internal<ScriptContext::CLIENT>(found->Function, command);
		break;
	case ScriptContext::SERVER:
		ModConCommandCallback_Internal<ScriptContext::SERVER>(found->Function, command);
		break;
	case ScriptContext::UI:
		ModConCommandCallback_Internal<ScriptContext::UI>(found->Function, command);
		break;
	};
}

void ModManager::LoadMods()
{
	// Unload mods first
	if (m_bHasLoadedMods)
		UnloadMods();

	// Ensure dirs exist
	fs::remove_all(GetCompiledAssetsPath());
	fs::create_directories(GetModFolderPath());
	fs::create_directories(GetThunderstoreModFolderPath());
	fs::create_directories(GetRemoteModFolderPath());

	// Read enabled mods cfg
	nlohmann::json jsEnabledModsCfg;
	bool bHasEnabledModsCfg = false;

	try
	{
		std::string svEnabledModsCfg;
		CFileStream fStream;

		if (fStream.Open(FormatA("%s/enabledmods.json", g_svProfileDir.c_str()).c_str(), CFileStream::READ))
		{
			fStream.ReadString(svEnabledModsCfg);
			fStream.Close();

			jsEnabledModsCfg = nlohmann::json::parse(svEnabledModsCfg);

			bHasEnabledModsCfg = true;
		}
	}
	catch (const std::exception& ex)
	{
		NOTE_UNUSED(ex);
		Error(eLog::MODSYS, NO_ERROR, "Failed to parse enabledmods.json\n");
	}

	std::vector<fs::path> vModDirs;

	// Get mod directories
	fs::directory_iterator fsClassicModsDir = fs::directory_iterator(GetModFolderPath());
	fs::directory_iterator fsRemoteModsDir = fs::directory_iterator(GetRemoteModFolderPath());

	// Collect all mods in classic and remote mod dirs
	for (fs::directory_iterator modIterator : {fsClassicModsDir, fsRemoteModsDir})
		for (fs::directory_entry dir : modIterator)
			if (FileExists(dir.path() / "mod.json"))
				vModDirs.push_back(dir.path());

	// Collect all thunderstore mods and make sure the package dir matches AUTHOR-MOD-VERSION
	fs::directory_iterator thunderstoreModsDir = fs::directory_iterator(GetThunderstoreModFolderPath());
	std::regex pattern(R"(.*\\([a-zA-Z0-9_]+)-([a-zA-Z0-9_]+)-(\d+\.\d+\.\d+))");
	for (fs::directory_entry dir : thunderstoreModsDir)
	{
		fs::path modsDir = dir.path() / "mods"; // Check for mods folder in the Thunderstore mod
		// Use regex to match `AUTHOR-MOD-VERSION` pattern
		if (!std::regex_match(dir.path().string(), pattern))
		{
			Warning(eLog::MODSYS, "The following directory did not match 'AUTHOR-MOD-VERSION': %s\n", dir.path().string().c_str());
			continue; // skip loading mod that doesn't match
		}
		if (FileExists(modsDir) && fs::is_directory(modsDir))
		{
			for (fs::directory_entry subDir : fs::directory_iterator(modsDir))
			{
				if (FileExists(subDir.path() / "mod.json"))
				{
					vModDirs.push_back(subDir.path());
				}
			}
		}
	}

	// Try to load all collected mods
	for (fs::path modDir : vModDirs)
	{
		// First read mod.json
		std::string svModJson;
		CFileStream fStream;
		if (fStream.Open(FormatA("%s/mod.json", modDir.string().c_str()).c_str(), CFileStream::READ))
		{
			fStream.ReadString(svModJson);
			fStream.Close();
		}
		else
		{
			Warning(eLog::MODSYS, "Mod file at '%s' does not exist or could not be read, is it installed correctly?\n", (modDir / "mod.json").string().c_str());
			continue;
		}

		// Parse mod.json
		Mod mod(modDir, svModJson);

		// Setup mod dependencies
		for (auto& pair : mod.DependencyConstants)
		{
			if (m_DependencyConstants.find(pair.first) != m_DependencyConstants.end() && m_DependencyConstants[pair.first] != pair.second)
			{
				Error(eLog::MODSYS, NO_ERROR,
					  "'%s' attempted to register a dependency constant '%s' for '%s' that already exists for '%s'. "
					  "Change the constant name.\n",
					  mod.Name.c_str(), pair.first.c_str(), pair.second.c_str(), m_DependencyConstants[pair.first].c_str());
				mod.m_bWasReadSuccessfully = false;
				break;
			}
			if (m_DependencyConstants.find(pair.first) == m_DependencyConstants.end())
				m_DependencyConstants.emplace(pair);
		}

		// Check if it should be enabled
		if (bHasEnabledModsCfg)
		{
			try
			{
				mod.m_bEnabled = jsEnabledModsCfg.value(mod.Name, true);
			}
			catch (const std::exception& ex)
			{
				NOTE_UNUSED(ex);
				mod.m_bEnabled = true;
				Error(eLog::MODSYS, NO_ERROR, "enabledmods.json: '%s' has incorrect type ( expected bool )\n", mod.Name.c_str());
			}
		}
		else
		{
			mod.m_bEnabled = true;
		}

		// If succesful add it to m_LoadedMods, otherwise don't
		if (mod.m_bWasReadSuccessfully)
		{
			if (mod.m_bEnabled)
				DevMsg(eLog::MODSYS, "'%s' loaded successfully\n", mod.Name.c_str());
			else
				DevMsg(eLog::MODSYS, "'%s' loaded successfully (DISABLED)\n", mod.Name.c_str());

			m_LoadedMods.push_back(mod);
		}
		else
		{
			Warning(eLog::MODSYS, "Mod file at '%s' failed to load\n", (modDir / "mod.json").string().c_str());
		}
	}

	// sort by load prio, lowest-highest
	std::sort(m_LoadedMods.begin(), m_LoadedMods.end(), [](Mod& a, Mod& b) { return a.LoadPriority < b.LoadPriority; });

	for (Mod& mod : m_LoadedMods)
	{
		if (!mod.m_bEnabled)
			continue;

		// register convars
		// for reloads, this is sorta barebones, when we have a good findconvar method, we could probably reset flags and stuff on
		// preexisting convars note: we don't delete convars if they already exist because they're used for script stuff, unfortunately this
		// causes us to leak memory on reload, but not much, potentially find a way to not do this at some point
		for (ModConVar* convar : mod.ConVars)
		{
			// make sure convar isn't registered yet, unsure if necessary but idk what
			// behaviour is for defining same convar multiple times
			if (!g_pCVar->FindVar(convar->Name.c_str()))
			{
				ConVar::StaticCreate(convar->Name.c_str(), convar->DefaultValue.c_str(), convar->Flags, convar->HelpString.c_str());
			}
		}

		for (ModConCommand* command : mod.ConCommands)
		{
			// make sure command isnt't registered multiple times.
			if (!g_pCVar->FindCommand(command->Name.c_str()))
			{
				ConCommand::StaticCreate(command->Name.c_str(), command->HelpString.c_str(), command->Flags, ModConCommandCallback, nullptr);
			}
		}

		// read vpk paths
		if (FileExists(mod.m_ModDirectory / "vpk"))
		{
			// Try to read the cfg if it exists
			nlohmann::json jsVpk;
			bool bHasVpkCfg = false;

			CFileStream fStream;
			if (fStream.Open(mod.m_ModDirectory / "vpk/vpk.json", CFileStream::READ))
			{
				std::string svVpk;
				fStream.ReadString(svVpk);
				fStream.Close();

				try
				{
					jsVpk = nlohmann::json::parse(svVpk);
					bHasVpkCfg = true;
				}
				catch (const std::exception& ex)
				{
					NOTE_UNUSED(ex);
				}
			}

			for (fs::directory_entry file : fs::directory_iterator(mod.m_ModDirectory / "vpk"))
			{
				// a bunch of checks to make sure we're only adding dir vpks and their paths are good
				// note: the game will literally only load vpks with the english prefix
				if (fs::is_regular_file(file) && file.path().extension() == ".vpk" && file.path().string().find("english") != std::string::npos && file.path().string().find(".bsp.pak000_dir") != std::string::npos)
				{
					std::string formattedPath = file.path().filename().string();

					// this really fucking sucks but it'll work
					std::string vpkName = formattedPath.substr(strlen("english"), formattedPath.find(".bsp") - 3);

					ModVPKEntry modVpk;
					modVpk.m_bAutoLoad = !bHasVpkCfg || (jsVpk.contains("Preload") && jsVpk["Preload"].contains(vpkName) && jsVpk["Preload"][vpkName].is_boolean() && jsVpk["Preload"][vpkName].get<bool>());
					modVpk.m_sVpkPath = (file.path().parent_path() / vpkName).string();

					mod.Vpks.emplace_back(modVpk);

					if (m_bHasLoadedMods && modVpk.m_bAutoLoad)
						g_pFilesystem->m_vtable->MountVPK(g_pFilesystem, vpkName.c_str());
				}
			}
		}

		// read rpak paths
		if (FileExists(mod.m_ModDirectory / "paks"))
		{
			// Try to read the cfg if it exists
			nlohmann::json jsRPak;
			bool bHasRPakCfg = false;

			CFileStream fStream;
			if (fStream.Open(mod.m_ModDirectory / "paks/rpak.json", CFileStream::READ))
			{
				std::string svRPak;
				fStream.ReadString(svRPak);
				fStream.Close();

				try
				{
					jsRPak = nlohmann::json::parse(svRPak);
					bHasRPakCfg = true;
				}
				catch (const std::exception& ex)
				{
					NOTE_UNUSED(ex);
				}
			}

			// read pak aliases
			if (bHasRPakCfg && jsRPak.contains("Aliases"))
			{
				for (auto& jsAlias : jsRPak["Aliases"].items())
				{
					// TODO [Fifty]: Log this
					if (!jsAlias.value().is_string())
						continue;

					mod.RpakAliases.insert(std::make_pair(jsAlias.key(), jsAlias.value().get<std::string>()));
				}
			}

			for (fs::directory_entry file : fs::directory_iterator(mod.m_ModDirectory / "paks"))
			{
				// ensure we're only loading rpaks
				if (fs::is_regular_file(file) && file.path().extension() == ".rpak")
				{
					std::string pakName(file.path().filename().string());

					ModRpakEntry modPak;
					modPak.m_bAutoLoad = !bHasRPakCfg || (jsRPak.contains("Preload") && jsRPak["Preload"].is_object() && jsRPak["Preload"].contains(pakName) && jsRPak["Preload"][pakName].is_boolean() && jsRPak["Preload"][pakName].get<bool>());

					// postload things
					if (!bHasRPakCfg || (jsRPak.contains("Postload") && jsRPak["Postload"].is_object() && jsRPak["Postload"].contains(pakName) && jsRPak["Postload"][pakName].is_string()))
						modPak.m_sLoadAfterPak = jsRPak["Postload"][pakName].get<std::string>();

					modPak.m_sPakName = pakName;

					mod.Rpaks.emplace_back(modPak);

					// read header of file and get the starpak paths
					// this is done here as opposed to on starpak load because multiple rpaks can load a starpak
					// and there is seemingly no good way to tell which rpak is causing the load of a starpak :/

					std::ifstream rpakStream(file.path(), std::ios::binary);

					// seek to the point in the header where the starpak reference size is
					rpakStream.seekg(0x38, std::ios::beg);
					int starpaksSize = 0;
					rpakStream.read((char*)&starpaksSize, 2);

					// seek to just after the header
					rpakStream.seekg(0x58, std::ios::beg);
					// read the starpak reference(s)
					std::vector<char> buf(starpaksSize);
					rpakStream.read(buf.data(), starpaksSize);

					rpakStream.close();

					// split the starpak reference(s) into strings to hash
					std::string str = "";
					for (int i = 0; i < starpaksSize; i++)
					{
						// if the current char is null, that signals the end of the current starpak path
						if (buf[i] != 0x00)
						{
							str += buf[i];
						}
						else
						{
							// only add the string we are making if it isnt empty
							if (!str.empty())
							{
								mod.StarpakPaths.push_back(STR_HASH(str));
								DevMsg(eLog::MODSYS, "Mod %s registered starpak '%s'\n", mod.Name.c_str(), str.c_str());
								str = "";
							}
						}
					}

					// not using atm because we need to resolve path to rpak
					// if (m_hasLoadedMods && modPak.m_bAutoLoad)
					//	g_pPakLoadManager->LoadPakAsync(pakName.c_str());
				}
			}
		}

		// read keyvalues paths
		if (FileExists(mod.m_ModDirectory / "keyvalues"))
		{
			for (fs::directory_entry file : fs::recursive_directory_iterator(mod.m_ModDirectory / "keyvalues"))
			{
				if (fs::is_regular_file(file))
				{
					std::string kvStr = g_pModManager->NormaliseModFilePath(file.path().lexically_relative(mod.m_ModDirectory / "keyvalues"));
					mod.KeyValues.emplace(STR_HASH(kvStr), kvStr);
				}
			}
		}

		// read pdiff
		if (FileExists(mod.m_ModDirectory / "mod.pdiff"))
		{
			std::ifstream pdiffStream(mod.m_ModDirectory / "mod.pdiff");

			if (!pdiffStream.fail())
			{
				std::stringstream pdiffStringStream;
				while (pdiffStream.peek() != EOF)
					pdiffStringStream << (char)pdiffStream.get();

				pdiffStream.close();

				mod.Pdiff = pdiffStringStream.str();
			}
		}

		// read bink video paths
		if (FileExists(mod.m_ModDirectory / "media"))
		{
			for (fs::directory_entry file : fs::recursive_directory_iterator(mod.m_ModDirectory / "media"))
				if (fs::is_regular_file(file) && file.path().extension() == ".bik")
					mod.BinkVideos.push_back(file.path().filename().string());
		}

		// try to load audio
		if (FileExists(mod.m_ModDirectory / "audio"))
		{
			for (fs::directory_entry file : fs::directory_iterator(mod.m_ModDirectory / "audio"))
			{
				if (fs::is_regular_file(file) && file.path().extension().string() == ".json")
				{
					if (!g_CustomAudioManager.TryLoadAudioOverride(file.path()))
					{
						Warning(eLog::MODSYS, "Mod %s has an invalid audio def %s\n", mod.Name.c_str(), file.path().filename().string().c_str());
						continue;
					}
				}
			}
		}
	}

	// in a seperate loop because we register mod files in reverse order, since mods loaded later should have their files prioritised
	for (int64_t i = m_LoadedMods.size() - 1; i > -1; i--)
	{
		if (!m_LoadedMods[i].m_bEnabled)
			continue;

		if (FileExists(m_LoadedMods[i].m_ModDirectory / MOD_OVERRIDE_DIR))
		{
			for (fs::directory_entry file : fs::recursive_directory_iterator(m_LoadedMods[i].m_ModDirectory / MOD_OVERRIDE_DIR))
			{
				std::string path = g_pModManager->NormaliseModFilePath(file.path().lexically_relative(m_LoadedMods[i].m_ModDirectory / MOD_OVERRIDE_DIR));
				if (file.is_regular_file() && m_ModFiles.find(path) == m_ModFiles.end())
				{
					ModOverrideFile modFile;
					modFile.m_pOwningMod = &m_LoadedMods[i];
					modFile.m_Path = path;
					m_ModFiles.insert(std::make_pair(path, modFile));
				}
			}
		}
	}

	m_bHasLoadedMods = true;

	ReloadMapsList();
}

void ModManager::UnloadMods()
{
	// clean up stuff from mods before we unload
	m_vMapList.clear();
	m_ModFiles.clear();
	m_DependencyConstants.clear();
	fs::remove_all(GetCompiledAssetsPath());

	g_CustomAudioManager.ClearAudioOverrides();

	nlohmann::json jsEnabledModsCfg;

	for (Mod& mod : m_LoadedMods)
	{
		// remove all built kvs
		for (std::pair<size_t, std::string> kvPaths : mod.KeyValues)
			fs::remove(GetCompiledAssetsPath() / fs::path(kvPaths.second).lexically_relative(mod.m_ModDirectory));

		mod.KeyValues.clear();

		// write to m_enabledModsCfg
		// FIXME [Fifty]: Should only be written when the user changes this, not when connecting to a server w/ remote mods
		jsEnabledModsCfg[mod.Name] = mod.m_bEnabled;
	}

	CFileStream fStream;
	if (fStream.Open(FormatA("%s/enabledmods.json", g_svProfileDir.c_str()).c_str(), CFileStream::WRITE))
	{
		fStream.WriteString(jsEnabledModsCfg.dump(4));
		fStream.Close();
	}

	// do we need to dealloc individual entries in m_loadedMods? idk, rework
	m_LoadedMods.clear();
}

std::string ModManager::NormaliseModFilePath(const fs::path path)
{
	std::string str = path.lexically_normal().string();

	// force to lowercase
	for (char& c : str)
		if (c <= 'Z' && c >= 'A')
			c = c - ('Z' - 'z');

	return str;
}

void ModManager::CompileAssetsForFile(const char* filename)
{
	size_t fileHash = STR_HASH(NormaliseModFilePath(fs::path(filename)));

	if (fileHash == m_hScriptsRsonHash)
		BuildScriptsRson();
	else if (fileHash == m_hPdefHash)
		BuildPdef();
	else if (fileHash == m_hKBActHash)
		BuildKBActionsList();
	else
	{
		// check if we should build keyvalues, depending on whether any of our mods have patch kvs for this file
		for (Mod& mod : m_LoadedMods)
		{
			if (!mod.m_bEnabled)
				continue;

			if (mod.KeyValues.find(fileHash) != mod.KeyValues.end())
			{
				TryBuildKeyValues(filename);
				return;
			}
		}
	}
}

void ModManager::ReloadMapsList()
{
	for (auto& modFilePair : m_ModFiles)
	{
		ModOverrideFile file = modFilePair.second;
		if (file.m_Path.extension() == ".bsp" && file.m_Path.parent_path().string() == "maps") // only allow mod maps actually in /maps atm
		{
			MapVPKInfo_t& map = m_vMapList.emplace_back();
			map.svName = file.m_Path.stem().string();
			map.svParent = file.m_pOwningMod->Name;
			map.eSource = ModManager::MOD;
		}
	}

	// get maps in vpk
	{
		const int iNumRetailNonMapVpks = 1;
		static const char* const ppRetailNonMapVpks[] = {"englishclient_frontend.bsp.pak000_dir.vpk"}; // don't include mp_common here as it contains mp_lobby

		// matches directory vpks, and captures their map name in the first group
		static const std::regex rVpkMapRegex("englishclient_([a-zA-Z0-9_]+)\\.bsp\\.pak000_dir\\.vpk", std::regex::icase);

		for (fs::directory_entry file : fs::directory_iterator("./vpk"))
		{
			std::string pathString = file.path().filename().string();

			bool bIsValidMapVpk = true;
			for (int i = 0; i < iNumRetailNonMapVpks; i++)
			{
				if (!pathString.compare(ppRetailNonMapVpks[i]))
				{
					bIsValidMapVpk = false;
					break;
				}
			}

			if (!bIsValidMapVpk)
				continue;

			// run our map vpk regex on the filename
			std::smatch match;
			std::regex_match(pathString, match, rVpkMapRegex);

			if (match.length() < 2)
				continue;

			std::string mapName = match[1].str();
			// special case: englishclient_mp_common contains mp_lobby, so hardcode the name here
			if (mapName == "mp_common")
				mapName = "mp_lobby";

			MapVPKInfo_t& map = m_vMapList.emplace_back();
			map.svName = mapName;
			map.svParent = pathString;
			map.eSource = ModManager::VPK;
		}
	}

	// get maps in game dir
	for (fs::directory_entry file : fs::directory_iterator(fmt::format("{}/maps", "r2"))) // assume mod dir
	{
		if (file.path().extension() == ".bsp")
		{
			MapVPKInfo_t& map = m_vMapList.emplace_back();
			map.svName = file.path().stem().string();
			map.svParent = "R2";
			map.eSource = ModManager::GAMEDIR;
		}
	}
}

fs::path GetModFolderPath()
{
	return fs::path(g_svProfileDir + MOD_FOLDER_SUFFIX);
}
fs::path GetThunderstoreModFolderPath()
{
	return fs::path(g_svProfileDir + THUNDERSTORE_MOD_FOLDER_SUFFIX);
}
fs::path GetRemoteModFolderPath()
{
	return fs::path(g_svProfileDir + REMOTE_MOD_FOLDER_SUFFIX);
}
fs::path GetCompiledAssetsPath()
{
	return fs::path(g_svProfileDir + COMPILED_ASSETS_SUFFIX);
}

ON_DLL_LOAD_RELIESON("engine.dll", ModManager, ConVar, (CModule module))
{
	g_pModManager = new ModManager;
}
