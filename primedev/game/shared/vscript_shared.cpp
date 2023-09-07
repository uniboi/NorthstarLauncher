#include "game/shared/vscript_shared.h"

#include "squirrel/squirrelautobind.h"
#include "squirrel/squirrelclasstypes.h"
#include "squirrel/squirreldatatypes.h"
#include "squirrel/squirrel.h"

#include "mods/modsavefiles.h"
#include "mods/modmanager.h"
#include "rtech/datatable.h"
#include "rtech/pakapi.h"
#include "filesystem/basefilesystem.h"
#include "originsdk/origin.h"

// void NSSaveFile( string file, string data )
ADD_SQFUNC("void", NSSaveFile, "string file, string data", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);
	if (mod == nullptr)
	{
		g_pSquirrel<context>->raiseerror(sqvm, "Has to be called from a mod function!");
		return SQRESULT_ERROR;
	}

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = g_pSquirrel<context>->getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   fileName, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	std::string content = g_pSquirrel<context>->getstring(sqvm, 2);
	if (ContainsInvalidChars(content))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File contents may not contain NUL/\\0 characters! Make sure your strings are valid!", mod->Name).c_str());
		return SQRESULT_ERROR;
	}

	fs::create_directories(dir);
	// this actually allows mods to go over the limit, but not by much
	// the limit is to prevent mods from taking gigabytes of space,
	// this ain't a cloud service.
	if (GetSizeOfFolderContentsMinusFile(dir, fileName) + content.length() > MAX_FOLDER_SIZE)
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("The mod {} has reached the maximum folder size.\n\nAsk the mod developer to optimize their data usage,"
														   "or increase the maximum folder size using the -maxfoldersize launch parameter.",
														   mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	g_pSaveFileManager->SaveFileAsync<context>(dir / fileName, content);

	return SQRESULT_NULL;
}

// void NSSaveJSONFile(string file, table data)
ADD_SQFUNC("void", NSSaveJSONFile, "string file, table data", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);
	if (mod == nullptr)
	{
		g_pSquirrel<context>->raiseerror(sqvm, "Has to be called from a mod function!");
		return SQRESULT_ERROR;
	}

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = g_pSquirrel<context>->getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   fileName, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	// Note - this cannot be done in the async func since the table may get garbage collected.
	// This means that especially large tables may still clog up the system.

	nlohmann::json jsContent;
	CScriptJson<context>::EncodeJsonTable(sqvm->_stackOfCurrentFunction[2]._VAL.asTable, jsContent);
	std::string svContent = jsContent.dump(4);

	if (ContainsInvalidChars(svContent))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File contents may not contain NUL/\\0 characters! Make sure your strings are valid!", mod->Name).c_str());
		return SQRESULT_ERROR;
	}

	fs::create_directories(dir);
	// this actually allows mods to go over the limit, but not by much
	// the limit is to prevent mods from taking gigabytes of space,
	// this ain't a cloud service.
	if (GetSizeOfFolderContentsMinusFile(dir, fileName) + svContent.length() > MAX_FOLDER_SIZE)
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("The mod {} has reached the maximum folder size.\n\nAsk the mod developer to optimize their data usage,"
														   "or increase the maximum folder size using the -maxfoldersize launch parameter.",
														   mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	g_pSaveFileManager->SaveFileAsync<context>(dir / fileName, svContent);

	return SQRESULT_NULL;
}

// int NS_InternalLoadFile(string file)
ADD_SQFUNC("int", NS_InternalLoadFile, "string file", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm, 1); // the function that called NSLoadFile :)
	if (mod == nullptr)
	{
		g_pSquirrel<context>->raiseerror(sqvm, "Has to be called from a mod function!");
		return SQRESULT_ERROR;
	}

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = g_pSquirrel<context>->getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   fileName, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushinteger(sqvm, g_pSaveFileManager->LoadFileAsync<context>(dir / fileName));

	return SQRESULT_NOTNULL;
}

// bool NSDoesFileExist(string file)
ADD_SQFUNC("bool", NSDoesFileExist, "string file", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = g_pSquirrel<context>->getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   fileName, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushbool(sqvm, FileExists(dir / (fileName)));
	return SQRESULT_NOTNULL;
}

// int NSGetFileSize(string file)
ADD_SQFUNC("int", NSGetFileSize, "string file", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = g_pSquirrel<context>->getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   fileName, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}
	try
	{
		// throws if file does not exist
		// we don't want stuff such as "file does not exist, file is unavailable" to be lethal, so we just try/catch fs errors
		g_pSquirrel<context>->pushinteger(sqvm, (int)(fs::file_size(dir / fileName) / 1024));
	}
	catch (fs::filesystem_error const& ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "GET FILE SIZE FAILED! Is the path valid?\n");
		g_pSquirrel<context>->raiseerror(sqvm, ex.what());
		return SQRESULT_ERROR;
	}
	return SQRESULT_NOTNULL;
}

// void NSDeleteFile(string file)
ADD_SQFUNC("void", NSDeleteFile, "string file", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = g_pSquirrel<context>->getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   fileName, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}

	g_pSaveFileManager->DeleteFileAsync<context>(dir / fileName);
	return SQRESULT_NOTNULL;
}

// The param is not optional because that causes issues :)
ADD_SQFUNC("array<string>", NS_InternalGetAllFiles, "string path", "", ScriptContext::CLIENT | ScriptContext::UI | ScriptContext::SERVER)
{
	// depth 1 because this should always get called from Northstar.Custom
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm, 1);
	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string pathStr = g_pSquirrel<context>->getstring(sqvm, 1);
	fs::path path = dir;
	if (pathStr != "")
		path = dir / pathStr;
	if (!IsPathSafe(pathStr, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   pathStr, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}
	try
	{
		g_pSquirrel<context>->newarray(sqvm, 0);
		for (const auto& entry : fs::directory_iterator(path))
		{
			g_pSquirrel<context>->pushstring(sqvm, entry.path().filename().string().c_str());
			g_pSquirrel<context>->arrayappend(sqvm, -2);
		}
		return SQRESULT_NOTNULL;
	}
	catch (std::exception ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "DIR ITERATE FAILED! Is the path valid?\n");
		g_pSquirrel<context>->raiseerror(sqvm, ex.what());
		return SQRESULT_ERROR;
	}
}

ADD_SQFUNC("bool", NSIsFolder, "string path", "", ScriptContext::CLIENT | ScriptContext::UI | ScriptContext::SERVER)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);
	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string pathStr = g_pSquirrel<context>->getstring(sqvm, 1);
	fs::path path = dir;
	if (pathStr != "")
		path = dir / pathStr;
	if (!IsPathSafe(pathStr, dir))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("File name invalid ({})! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's "
														   "save folder.",
														   pathStr, mod->Name)
												   .c_str());
		return SQRESULT_ERROR;
	}
	try
	{
		g_pSquirrel<context>->pushbool(sqvm, fs::is_directory(path));
		return SQRESULT_NOTNULL;
	}
	catch (std::exception ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "DIR READ FAILED! Is the path valid?\n");
		DevMsg(eLog::MODSYS, "%s\n", path.string().c_str());
		g_pSquirrel<context>->raiseerror(sqvm, ex.what());
		return SQRESULT_ERROR;
	}
}

// side note, expensive.
ADD_SQFUNC("int", NSGetTotalSpaceRemaining, "", "", ScriptContext::CLIENT | ScriptContext::UI | ScriptContext::SERVER)
{
	Mod* mod = g_pSquirrel<context>->getcallingmod(sqvm);
	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	g_pSquirrel<context>->pushinteger(sqvm, (MAX_FOLDER_SIZE - GetSizeOfFolder(dir)) / 1024);
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("array<string>", NSGetModNames, "", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	g_pSquirrel<context>->newarray(sqvm, 0);

	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		g_pSquirrel<context>->pushstring(sqvm, mod.Name.c_str());
		g_pSquirrel<context>->arrayappend(sqvm, -2);
	}

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("bool", NSIsModEnabled, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			g_pSquirrel<context>->pushbool(sqvm, mod.m_bEnabled);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("void", NSSetModEnabled, "string modName, bool enabled", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);
	const SQBool enabled = g_pSquirrel<context>->getbool(sqvm, 2);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			mod.m_bEnabled = enabled;
			return SQRESULT_NULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("string", NSGetModDescriptionByModName, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			g_pSquirrel<context>->pushstring(sqvm, mod.Description.c_str());
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("string", NSGetModVersionByModName, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			g_pSquirrel<context>->pushstring(sqvm, mod.Version.c_str());
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("string", NSGetModDownloadLinkByModName, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			g_pSquirrel<context>->pushstring(sqvm, mod.DownloadLink.c_str());
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("int", NSGetModLoadPriority, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			g_pSquirrel<context>->pushinteger(sqvm, mod.LoadPriority);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("bool", NSIsModRequiredOnClient, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			g_pSquirrel<context>->pushbool(sqvm, mod.RequiredOnClient);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("array<string>", NSGetModConvarsByModName, "string modName", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	const SQChar* modName = g_pSquirrel<context>->getstring(sqvm, 1);
	g_pSquirrel<context>->newarray(sqvm, 0);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			for (ModConVar* cvar : mod.ConVars)
			{
				g_pSquirrel<context>->pushstring(sqvm, cvar->Name.c_str());
				g_pSquirrel<context>->arrayappend(sqvm, -2);
			}

			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NOTNULL; // return empty array
}

ADD_SQFUNC("void", NSReloadMods, "", "", ScriptContext::SERVER | ScriptContext::CLIENT | ScriptContext::UI)
{
	g_pModManager->LoadMods();
	return SQRESULT_NULL;
}

ADD_SQFUNC("table", DecodeJSON, "string json, bool fatalParseErrors = false", "converts a json string to a squirrel table", ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER)
{
	const char* pJson = g_pSquirrel<context>->getstring(sqvm, 1);
	const bool bFatalParseErrors = g_pSquirrel<context>->getbool(sqvm, 2);

	nlohmann::json jsObj;

	try
	{
		jsObj = nlohmann::json::parse(pJson);
	}
	catch (const std::exception& ex)
	{
		g_pSquirrel<context>->newtable(sqvm);

		std::string svError = FormatA("Failed parsing json file: encountered parse error: %s", ex.what());

		if (bFatalParseErrors)
		{
			g_pSquirrel<context>->raiseerror(sqvm, svError.c_str());
			return SQRESULT_ERROR;
		}

		Warning(eLog::NS, "%s\n", svError.c_str());
		return SQRESULT_NOTNULL;
	}

	CScriptJson<context>::DecodeJson(sqvm, jsObj);

	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("string", EncodeJSON, "table data", "converts a squirrel table to a json string", ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER)
{
	nlohmann::json jsObj;
	CScriptJson<context>::EncodeJsonTable(sqvm->_stackOfCurrentFunction[1]._VAL.asTable, jsObj);

	std::string svObj = jsObj.dump();
	g_pSquirrel<context>->pushstring(sqvm, svObj.c_str(), -1);
	return SQRESULT_NOTNULL;
}

// asset function StringToAsset( string assetName )
ADD_SQFUNC("asset", StringToAsset, "string assetName", "converts a given string to an asset", ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER)
{
	g_pSquirrel<context>->pushasset(sqvm, g_pSquirrel<context>->getstring(sqvm, 1), -1);
	return SQRESULT_NOTNULL;
}

// string function NSGetLocalPlayerUID()
ADD_SQFUNC("string", NSGetLocalPlayerUID, "", "Returns the local player's uid.", ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER)
{
	if (g_pLocalPlayerUserID)
	{
		g_pSquirrel<context>->pushstring(sqvm, g_pLocalPlayerUserID);
		return SQRESULT_NOTNULL;
	}

	return SQRESULT_NULL;
}

ADD_SQFUNC("string", NSGetCurrentModName, "", "Returns the mod name of the script running this function", ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER)
{
	int depth = g_pSquirrel<context>->getinteger(sqvm, 1);
	if (auto mod = g_pSquirrel<context>->getcallingmod(sqvm, depth); mod == nullptr)
	{
		g_pSquirrel<context>->raiseerror(sqvm, "NSGetModName was called from a non-mod script. This shouldn't be possible");
		return SQRESULT_ERROR;
	}
	else
	{
		g_pSquirrel<context>->pushstring(sqvm, mod->Name.c_str());
	}
	return SQRESULT_NOTNULL;
}

ADD_SQFUNC("string", NSGetCallingModName, "int depth = 0", "Returns the mod name of the script running this function", ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER)
{
	int depth = g_pSquirrel<context>->getinteger(sqvm, 1);
	if (auto mod = g_pSquirrel<context>->getcallingmod(sqvm, depth); mod == nullptr)
	{
		g_pSquirrel<context>->pushstring(sqvm, "Unknown");
	}
	else
	{
		g_pSquirrel<context>->pushstring(sqvm, mod->Name.c_str());
	}
	return SQRESULT_NOTNULL;
}

// var function GetDataTable( asset path )
REPLACE_SQFUNC(GetDataTable, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	static std::unordered_map<std::string, CSVData> CSVCache;

	const char* pAssetName;
	g_pSquirrel<context>->getasset(sqvm, 2, &pAssetName);

	if (strncmp(pAssetName, "datatable/", 10))
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("Asset \"{}\" doesn't start with \"datatable/\"", pAssetName).c_str());
		return SQRESULT_ERROR;
	}
	else if (!Cvar_ns_prefer_datatable_from_disk->GetBool() && g_pPakLoadManager->LoadFile(pAssetName))
		return g_pSquirrel<context>->m_funcOriginals["GetDataTable"](sqvm);
	// either we prefer disk datatables, or we're loading a datatable that wasn't found in rpak
	else
	{
		std::string sAssetPath(fmt::format("scripts/{}", pAssetName));

		// first, check the cache
		if (CSVCache.find(pAssetName) != CSVCache.end())
		{
			CSVData** pUserdata = g_pSquirrel<context>->template createuserdata<CSVData*>(sqvm, sizeof(CSVData*));
			g_pSquirrel<context>->setuserdatatypeid(sqvm, -1, USERDATA_TYPE_DATATABLE_CUSTOM);
			*pUserdata = &CSVCache[pAssetName];

			return SQRESULT_NOTNULL;
		}

		// check files on disk
		// we don't use .rpak as the extension for on-disk datatables, so we need to replace .rpak with .csv in the filename we're reading
		fs::path diskAssetPath("scripts");
		if (fs::path(pAssetName).extension() == ".rpak")
			diskAssetPath /= fs::path(pAssetName).remove_filename() / (fs::path(pAssetName).stem().string() + ".csv");
		else
			diskAssetPath /= fs::path(pAssetName);

		std::string sDiskAssetPath(diskAssetPath.string());
		if (g_pFilesystem->m_vtable2->FileExists(&g_pFilesystem->m_vtable2, sDiskAssetPath.c_str(), "GAME"))
		{
			std::string sTableCSV = ReadVPKFile(sDiskAssetPath.c_str());
			if (!sTableCSV.size())
			{
				g_pSquirrel<context>->raiseerror(sqvm, fmt::format("Datatable \"{}\" is empty", pAssetName).c_str());
				return SQRESULT_ERROR;
			}

			// somewhat shit, but ensure we end with a newline to make parsing easier
			if (sTableCSV[sTableCSV.length() - 1] != '\n')
				sTableCSV += '\n';

			CSVData csv;
			csv.m_sAssetName = pAssetName;
			csv.m_sCSVString = sTableCSV;
			csv.m_nDataBufSize = sTableCSV.size();
			csv.m_pDataBuf = new char[csv.m_nDataBufSize];
			memcpy(csv.m_pDataBuf, &sTableCSV[0], csv.m_nDataBufSize);

			// parse the csv
			// csvs are essentially comma and newline-deliniated sets of strings for parsing, only thing we need to worry about is quoted
			// entries when we parse an element of the csv, rather than allocating an entry for it, we just convert that element to a
			// null-terminated string i.e., store the ptr to the first char of it, then make the comma that delinates it a nullchar

			bool bHasColumns = false;
			bool bInQuotes = false;

			std::vector<char*> vCurrentRow;
			char* pElemStart = csv.m_pDataBuf;
			char* pElemEnd = nullptr;

			for (int i = 0; i < csv.m_nDataBufSize; i++)
			{
				if (csv.m_pDataBuf[i] == '\r' && csv.m_pDataBuf[i + 1] == '\n')
				{
					if (!pElemEnd)
						pElemEnd = csv.m_pDataBuf + i;

					continue; // next iteration can handle the \n
				}

				// newline, end of a row
				if (csv.m_pDataBuf[i] == '\n')
				{
					// shouldn't have newline in string
					if (bInQuotes)
					{
						g_pSquirrel<context>->raiseerror(sqvm, "Unexpected \\n in string");
						return SQRESULT_ERROR;
					}

					// push last entry to current row
					if (pElemEnd)
						*pElemEnd = '\0';
					else
						csv.m_pDataBuf[i] = '\0';

					vCurrentRow.push_back(pElemStart);

					// newline, push last line to csv data and go from there
					if (!bHasColumns)
					{
						bHasColumns = true;
						csv.columns = vCurrentRow;
					}
					else
						csv.dataPointers.push_back(vCurrentRow);

					vCurrentRow.clear();
					// put start of current element at char after newline
					pElemStart = csv.m_pDataBuf + i + 1;
					pElemEnd = nullptr;
				}
				// we're starting or ending a quoted string
				else if (csv.m_pDataBuf[i] == '"')
				{
					// start quoted string
					if (!bInQuotes)
					{
						// shouldn't have quoted strings in column names
						if (!bHasColumns)
						{
							g_pSquirrel<context>->raiseerror(sqvm, "Unexpected \" in column name");
							return SQRESULT_ERROR;
						}

						bInQuotes = true;
						// put start of current element at char after string begin
						pElemStart = csv.m_pDataBuf + i + 1;
					}
					// end quoted string
					else
					{
						pElemEnd = csv.m_pDataBuf + i;
						bInQuotes = false;
					}
				}
				// don't parse commas in quotes
				else if (bInQuotes)
				{
					continue;
				}
				// comma, push new entry to current row
				else if (csv.m_pDataBuf[i] == ',')
				{
					if (pElemEnd)
						*pElemEnd = '\0';
					else
						csv.m_pDataBuf[i] = '\0';

					vCurrentRow.push_back(pElemStart);
					// put start of next element at char after comma
					pElemStart = csv.m_pDataBuf + i + 1;
					pElemEnd = nullptr;
				}
			}

			// add to cache and return
			CSVData** pUserdata = g_pSquirrel<context>->template createuserdata<CSVData*>(sqvm, sizeof(CSVData*));
			g_pSquirrel<context>->setuserdatatypeid(sqvm, -1, USERDATA_TYPE_DATATABLE_CUSTOM);
			CSVCache[pAssetName] = csv;
			*pUserdata = &CSVCache[pAssetName];

			return SQRESULT_NOTNULL;
		}
		// the file doesn't exist on disk, check rpak if we haven't already
		else if (Cvar_ns_prefer_datatable_from_disk->GetBool() && g_pPakLoadManager->LoadFile(pAssetName))
			return g_pSquirrel<context>->m_funcOriginals["GetDataTable"](sqvm);
		// the file doesn't exist at all, error
		else
		{
			g_pSquirrel<context>->raiseerror(sqvm, fmt::format("Datatable {} not found", pAssetName).c_str());
			return SQRESULT_ERROR;
		}
	}
}

// int function GetDataTableColumnByName( var datatable, string columnName )
REPLACE_SQFUNC(GetDataTableColumnByName, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableColumnByName"](sqvm);

	CSVData* csv = *pData;
	const char* pColumnName = g_pSquirrel<context>->getstring(sqvm, 2);

	for (int i = 0; i < csv->columns.size(); i++)
	{
		if (!strcmp(csv->columns[i], pColumnName))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	// column not found
	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowCount( var datatable )
REPLACE_SQFUNC(GetDataTableRowCount, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDatatableRowCount"](sqvm);

	CSVData* csv = *pData;
	g_pSquirrel<context>->pushinteger(sqvm, csv->dataPointers.size());
	return SQRESULT_NOTNULL;
}

// string function GetDataTableString( var datatable, int row, int col )
REPLACE_SQFUNC(GetDataTableString, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableString"](sqvm);

	CSVData* csv = *pData;
	const int nRow = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nCol = g_pSquirrel<context>->getinteger(sqvm, 3);
	if (nRow >= csv->dataPointers.size() || nCol >= csv->dataPointers[nRow].size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("row {} and col {} are outside of range row {} and col {}", nRow, nCol, csv->dataPointers.size(), csv->columns.size()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushstring(sqvm, csv->dataPointers[nRow][nCol], -1);
	return SQRESULT_NOTNULL;
}

// asset function GetDataTableAsset( var datatable, int row, int col )
REPLACE_SQFUNC(GetDataTableAsset, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableAsset"](sqvm);

	CSVData* csv = *pData;
	const int nRow = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nCol = g_pSquirrel<context>->getinteger(sqvm, 3);
	if (nRow >= csv->dataPointers.size() || nCol >= csv->dataPointers[nRow].size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("row {} and col {} are outside of range row {} and col {}", nRow, nCol, csv->dataPointers.size(), csv->columns.size()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushasset(sqvm, csv->dataPointers[nRow][nCol], -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableInt( var datatable, int row, int col )
REPLACE_SQFUNC(GetDataTableInt, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableInt"](sqvm);

	CSVData* csv = *pData;
	const int nRow = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nCol = g_pSquirrel<context>->getinteger(sqvm, 3);
	if (nRow >= csv->dataPointers.size() || nCol >= csv->dataPointers[nRow].size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("row {} and col {} are outside of range row {} and col {}", nRow, nCol, csv->dataPointers.size(), csv->columns.size()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushinteger(sqvm, std::stoi(csv->dataPointers[nRow][nCol]));
	return SQRESULT_NOTNULL;
}

// float function GetDataTableFloat( var datatable, int row, int col )
REPLACE_SQFUNC(GetDataTableFloat, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableFloat"](sqvm);

	CSVData* csv = *pData;
	const int nRow = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nCol = g_pSquirrel<context>->getinteger(sqvm, 3);
	if (nRow >= csv->dataPointers.size() || nCol >= csv->dataPointers[nRow].size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("row {} and col {} are outside of range row {} and col {}", nRow, nCol, csv->dataPointers.size(), csv->columns.size()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushfloat(sqvm, std::stof(csv->dataPointers[nRow][nCol]));
	return SQRESULT_NOTNULL;
}

// bool function GetDataTableBool( var datatable, int row, int col )
REPLACE_SQFUNC(GetDataTableBool, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableBool"](sqvm);

	CSVData* csv = *pData;
	const int nRow = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nCol = g_pSquirrel<context>->getinteger(sqvm, 3);
	if (nRow >= csv->dataPointers.size() || nCol >= csv->dataPointers[nRow].size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("row {} and col {} are outside of range row {} and col {}", nRow, nCol, csv->dataPointers.size(), csv->columns.size()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushbool(sqvm, std::stoi(csv->dataPointers[nRow][nCol]));
	return SQRESULT_NOTNULL;
}

// vector function GetDataTableVector( var datatable, int row, int col )
REPLACE_SQFUNC(GetDataTableVector, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableVector"](sqvm);

	CSVData* csv = *pData;
	const int nRow = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nCol = g_pSquirrel<context>->getinteger(sqvm, 3);
	if (nRow >= csv->dataPointers.size() || nCol >= csv->dataPointers[nRow].size())
	{
		g_pSquirrel<context>->raiseerror(sqvm, fmt::format("row {} and col {} are outside of range row {} and col {}", nRow, nCol, csv->dataPointers.size(), csv->columns.size()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSquirrel<context>->pushvector(sqvm, StringToVector(csv->dataPointers[nRow][nCol]));
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowMatchingStringValue( var datatable, int col, string value )
REPLACE_SQFUNC(GetDataTableRowMatchingStringValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowMatchingStringValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const char* pStringVal = g_pSquirrel<context>->getstring(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (!strcmp(csv->dataPointers[i][nCol], pStringVal))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowMatchingAssetValue( var datatable, int col, asset value )
REPLACE_SQFUNC(GetDataTableMatchingAssetValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowMatchingAssetValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const char* pStringVal;
	g_pSquirrel<context>->getasset(sqvm, 3, &pStringVal);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (!strcmp(csv->dataPointers[i][nCol], pStringVal))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowMatchingFloatValue( var datatable, int col, float value )
REPLACE_SQFUNC(GetDataTableRowMatchingFloatValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowMatchingFloatValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const float flFloatVal = g_pSquirrel<context>->getfloat(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (flFloatVal == std::stof(csv->dataPointers[i][nCol]))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowMatchingIntValue( var datatable, int col, int value )
REPLACE_SQFUNC(GetDataTableRowMatchingIntValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowMatchingIntValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nIntVal = g_pSquirrel<context>->getinteger(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (nIntVal == std::stoi(csv->dataPointers[i][nCol]))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowMatchingVectorValue( var datatable, int col, vector value )
REPLACE_SQFUNC(GetDataTableRowMatchingVectorValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowMatchingVectorValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const Vector3 vVectorVal = g_pSquirrel<context>->getvector(sqvm, 3);

	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (vVectorVal == StringToVector(csv->dataPointers[i][nCol]))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowGreaterThanOrEqualToIntValue( var datatable, int col, int value )
REPLACE_SQFUNC(GetDataTableRowGreaterThanOrEqualToIntValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowGreaterThanOrEqualToIntValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nIntVal = g_pSquirrel<context>->getinteger(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (nIntVal >= std::stoi(csv->dataPointers[i][nCol]))
		{
			Warning(eLog::NS, "datatable not loaded\n");
			g_pSquirrel<context>->pushinteger(sqvm, 1);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowLessThanOrEqualToIntValue( var datatable, int col, int value )
REPLACE_SQFUNC(GetDataTableRowLessThanOrEqualToIntValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowLessThanOrEqualToIntValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const int nIntVal = g_pSquirrel<context>->getinteger(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (nIntVal <= std::stoi(csv->dataPointers[i][nCol]))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowGreaterThanOrEqualToFloatValue( var datatable, int col, float value )
REPLACE_SQFUNC(GetDataTableRowGreaterThanOrEqualToFloatValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowGreaterThanOrEqualToFloatValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const float flFloatVal = g_pSquirrel<context>->getfloat(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (flFloatVal >= std::stof(csv->dataPointers[i][nCol]))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

// int function GetDataTableRowLessThanOrEqualToFloatValue( var datatable, int col, float value )
REPLACE_SQFUNC(GetDataTableRowLessThanOrEqualToFloatValue, (ScriptContext::UI | ScriptContext::CLIENT | ScriptContext::SERVER))
{
	CSVData** pData;
	uint64_t typeId;
	g_pSquirrel<context>->getuserdata(sqvm, 2, &pData, &typeId);

	if (typeId != USERDATA_TYPE_DATATABLE_CUSTOM)
		return g_pSquirrel<context>->m_funcOriginals["GetDataTableRowLessThanOrEqualToFloatValue"](sqvm);

	CSVData* csv = *pData;
	int nCol = g_pSquirrel<context>->getinteger(sqvm, 2);
	const float flFloatVal = g_pSquirrel<context>->getfloat(sqvm, 3);
	for (int i = 0; i < csv->dataPointers.size(); i++)
	{
		if (flFloatVal <= std::stof(csv->dataPointers[i][nCol]))
		{
			g_pSquirrel<context>->pushinteger(sqvm, i);
			return SQRESULT_NOTNULL;
		}
	}

	g_pSquirrel<context>->pushinteger(sqvm, -1);
	return SQRESULT_NOTNULL;
}

//-----------------------------------------------------------------------------
// Purpose: Decodes a json object into a sq table
// Input  : *sqvm -
//          &jsObj -
//-----------------------------------------------------------------------------
template <ScriptContext context>
void CScriptJson<context>::DecodeJson(HSquirrelVM* sqvm, nlohmann::json& jsObj)
{
	// FIXME [Fifty]: This is stupid, make the sq func return a var so we dont have to do this
	if (jsObj.type() == nlohmann::json::value_t::array)
	{
		g_pSquirrel<context>->newtable(sqvm);
		g_pSquirrel<context>->pushstring(sqvm, "RootArray", -1);
		DecodeArray(sqvm, jsObj);
		g_pSquirrel<context>->newslot(sqvm, -3, false);
	}
	else
	{
		DecodeTable(sqvm, jsObj);
	}
}

//-----------------------------------------------------------------------------
// Purpose: Decodes a json object into a sq table
// Input  : *sqvm -
//          &jsObj -
//-----------------------------------------------------------------------------
template <ScriptContext context>
void CScriptJson<context>::DecodeTable(HSquirrelVM* sqvm, nlohmann::json& jsObj)
{
	g_pSquirrel<context>->newtable(sqvm);

	for (auto& js : jsObj.items())
	{
		switch (js.value().type())
		{
		case nlohmann::json::value_t::boolean:
			g_pSquirrel<context>->pushstring(sqvm, js.key().c_str(), -1);
			g_pSquirrel<context>->pushbool(sqvm, js.value().get<bool>());
			g_pSquirrel<context>->newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::string:
			g_pSquirrel<context>->pushstring(sqvm, js.key().c_str(), -1);
			g_pSquirrel<context>->pushstring(sqvm, js.value().get<std::string>().c_str());
			g_pSquirrel<context>->newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::number_integer:
		case nlohmann::json::value_t::number_unsigned:
			g_pSquirrel<context>->pushstring(sqvm, js.key().c_str(), -1);
			g_pSquirrel<context>->pushinteger(sqvm, js.value().get<int>());
			g_pSquirrel<context>->newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::number_float:
			g_pSquirrel<context>->pushstring(sqvm, js.key().c_str(), -1);
			g_pSquirrel<context>->pushfloat(sqvm, js.value().get<float>());
			g_pSquirrel<context>->newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::object:
			g_pSquirrel<context>->pushstring(sqvm, js.key().c_str(), -1);
			DecodeTable(sqvm, js.value());
			g_pSquirrel<context>->newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::array:
			g_pSquirrel<context>->pushstring(sqvm, js.key().c_str(), -1);
			DecodeArray(sqvm, js.value());
			g_pSquirrel<context>->newslot(sqvm, -3, false);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Decodes a json object into a sq array
// Input  : *sqvm -
//          &jsObj -
//-----------------------------------------------------------------------------
template <ScriptContext context>
void CScriptJson<context>::DecodeArray(HSquirrelVM* sqvm, nlohmann::json& jsObj)
{
	g_pSquirrel<context>->newarray(sqvm, 0);

	for (auto& js : jsObj)
	{
		switch (js.type())
		{
		case nlohmann::json::value_t::boolean:
			g_pSquirrel<context>->pushbool(sqvm, js.get<bool>());
			g_pSquirrel<context>->arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::string:
			g_pSquirrel<context>->pushstring(sqvm, js.get<std::string>().c_str(), -1);
			g_pSquirrel<context>->arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::number_integer:
		case nlohmann::json::value_t::number_unsigned:
			g_pSquirrel<context>->pushinteger(sqvm, js.get<int>());
			g_pSquirrel<context>->arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::number_float:
			g_pSquirrel<context>->pushfloat(sqvm, js.get<float>());
			g_pSquirrel<context>->arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::object:
			DecodeTable(sqvm, js);
			g_pSquirrel<context>->arrayappend(sqvm, -2);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Encodes json represented as a sq table into a json obj
// Input  : *pTable -
//          &jsObj -
//-----------------------------------------------------------------------------
template <ScriptContext context>
void CScriptJson<context>::EncodeJsonTable(SQTable* pTable, nlohmann::json& jsObj)
{
	for (int i = 0; i < pTable->_numOfNodes; i++)
	{
		tableNode* node = &pTable->_nodes[i];

		nlohmann::json jsTable;
		nlohmann::json jsArray;

		if (node->key._Type == OT_STRING)
		{
			switch (node->val._Type)
			{
			case OT_STRING:
				jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, node->val._VAL.asString->_val));
				break;
			case OT_INTEGER:
				jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, node->val._VAL.asInteger));
				break;
			case OT_FLOAT:
				jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, node->val._VAL.asFloat));
				break;
			case OT_BOOL:
				if (node->val._VAL.asInteger)
					jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, true));
				else
					jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, false));
				break;
			case OT_TABLE:
				EncodeJsonTable(node->val._VAL.asTable, jsTable);
				jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, jsTable));
				break;
			case OT_ARRAY:
				EncodeJsonArray(node->val._VAL.asArray, jsArray);
				jsObj.push_back(nlohmann::json::object_t::value_type(node->key._VAL.asString->_val, jsArray));
				break;
			default:
				Warning(eLog::NS, "CScriptJson::EncodeJsonTable: squirrel type %s not supported\n", SQTypeNameFromID(node->val._Type));
				break;
			}
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Encodes json represented as a sq array into a json obj
// Input  : *pArray -
//          &jsObj -
//-----------------------------------------------------------------------------
template <ScriptContext context>
void CScriptJson<context>::EncodeJsonArray(SQArray* pArray, nlohmann::json& jsObj)
{
	for (int i = 0; i < pArray->_usedSlots; i++)
	{
		SQObject* node = &pArray->_values[i];

		nlohmann::json jsTable;
		nlohmann::json jsArray;

		switch (node->_Type)
		{
		case OT_STRING:
			jsObj.push_back(node->_VAL.asString->_val);
			break;
		case OT_INTEGER:
			jsObj.push_back(node->_VAL.asInteger);
			break;
		case OT_FLOAT:
			jsObj.push_back(node->_VAL.asInteger);
			break;
		case OT_BOOL:
			if (node->_VAL.asInteger)
				jsObj.push_back(true);
			else
				jsObj.push_back(false);
			break;
		case OT_TABLE:
			EncodeJsonTable(node->_VAL.asTable, jsTable);
			jsObj.push_back(jsTable);
			break;
		case OT_ARRAY:
			EncodeJsonArray(node->_VAL.asArray, jsArray);
			jsObj.push_back(jsArray);
			break;
		default:
			Warning(eLog::NS, "CScriptJson::EncodeJsonArray: squirrel type %s not supported\n", SQTypeNameFromID(node->_Type));
		}
	}
}
