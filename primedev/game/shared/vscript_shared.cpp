#include "game/shared/vscript_shared.h"

#include "vscript/vscript.h"

#include "mods/modsavefiles.h"
#include "mods/modmanager.h"
#include "rtech/datatable.h"
#include "rtech/pakapi.h"
#include "filesystem/basefilesystem.h"
#include "originsdk/origin.h"
#include "tier2/curlutils.h"
#include "tier0/taskscheduler.h"

// void NSSaveFile( string file, string data )
SQRESULT Script_NSSaveFile(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);
	if (mod == nullptr)
	{
		sq_raiseerror(sqvm, "Has to be called from a mod function!");
		return SQRESULT_ERROR;
	}

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = sq_getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", fileName.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	std::string content = sq_getstring(sqvm, 2);
	if (ContainsInvalidChars(content))
	{
		sq_raiseerror(sqvm, FormatA("File contents may not contain NUL/\\0 characters! Make sure your strings are valid!").c_str());
		return SQRESULT_ERROR;
	}

	fs::create_directories(dir);
	// this actually allows mods to go over the limit, but not by much
	// the limit is to prevent mods from taking gigabytes of space,
	// this ain't a cloud service.
	if (GetSizeOfFolderContentsMinusFile(dir, fileName) + content.length() > MAX_FOLDER_SIZE)
	{
		sq_raiseerror(sqvm, FormatA("The mod %s has reached the maximum folder size.\n\nAsk the mod developer to optimize their data usage, or increase the maximum folder size using the -maxfoldersize launch parameter.", mod->Name.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSaveFileManager->SaveFileAsync(dir / fileName, content);

	return SQRESULT_NULL;
}

// void NSSaveJSONFile(string file, table data)
SQRESULT Script_NSSaveJSONFile(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);
	if (mod == nullptr)
	{
		sq_raiseerror(sqvm, "Has to be called from a mod function!");
		return SQRESULT_ERROR;
	}

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = sq_getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", fileName.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	// Note - this cannot be done in the async func since the table may get garbage collected.
	// This means that especially large tables may still clog up the system.

	nlohmann::json jsContent;
	CScriptJson::EncodeJsonTable(sqvm->_stackOfCurrentFunction[2]._VAL.asTable, jsContent);
	std::string svContent = jsContent.dump(4);

	if (ContainsInvalidChars(svContent))
	{
		sq_raiseerror(sqvm, FormatA("File contents may not contain NUL/\\0 characters! Make sure your strings are valid!").c_str());
		return SQRESULT_ERROR;
	}

	fs::create_directories(dir);
	// this actually allows mods to go over the limit, but not by much
	// the limit is to prevent mods from taking gigabytes of space,
	// this ain't a cloud service.
	if (GetSizeOfFolderContentsMinusFile(dir, fileName) + svContent.length() > MAX_FOLDER_SIZE)
	{
		sq_raiseerror(sqvm, FormatA("The mod %s has reached the maximum folder size.\n\nAsk the mod developer to optimize their data usage, or increase the maximum folder size using the -maxfoldersize launch parameter.", mod->Name.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSaveFileManager->SaveFileAsync(dir / fileName, svContent);

	return SQRESULT_NULL;
}

// int NS_InternalLoadFile(string file)
SQRESULT Script_NS_InternalLoadFile(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm, 1); // the function that called NSLoadFile :)
	if (mod == nullptr)
	{
		sq_raiseerror(sqvm, "Has to be called from a mod function!");
		return SQRESULT_ERROR;
	}

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = sq_getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", fileName.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	sq_pushinteger(sqvm, g_pSaveFileManager->LoadFileAsync(dir / fileName, (ScriptContext)sqvm->sharedState->cSquirrelVM->vmContext));

	return SQRESULT_NOTNULL;
}

// bool NSDoesFileExist(string file)
SQRESULT Script_NSDoesFileExist(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = sq_getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", fileName.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	sq_pushbool(sqvm, FileExists(dir / (fileName)));
	return SQRESULT_NOTNULL;
}

// int NSGetFileSize(string file)
SQRESULT Script_NSGetFileSize(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = sq_getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", fileName.c_str()).c_str());
		return SQRESULT_ERROR;
	}
	try
	{
		// throws if file does not exist
		// we don't want stuff such as "file does not exist, file is unavailable" to be lethal, so we just try/catch fs errors
		sq_pushinteger(sqvm, (int)(fs::file_size(dir / fileName) / 1024));
	}
	catch (fs::filesystem_error const& ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "GET FILE SIZE FAILED! Is the path valid?\n");
		sq_raiseerror(sqvm, ex.what());
		return SQRESULT_ERROR;
	}
	return SQRESULT_NOTNULL;
}

// void NSDeleteFile(string file)
SQRESULT Script_NSDeleteFile(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);

	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string fileName = sq_getstring(sqvm, 1);
	if (!IsPathSafe(fileName, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", fileName.c_str()).c_str());
		return SQRESULT_ERROR;
	}

	g_pSaveFileManager->DeleteFileAsync(dir / fileName);
	return SQRESULT_NOTNULL;
}

// The param is not optional because that causes issues :)
SQRESULT Script_NS_InternalGetAllFiles(HSQUIRRELVM sqvm)
{
	// depth 1 because this should always get called from Northstar.Custom
	Mod* mod = sq_getcallingmod(sqvm, 1);
	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string pathStr = sq_getstring(sqvm, 1);
	fs::path path = dir;
	if (pathStr != "")
		path = dir / pathStr;
	if (!IsPathSafe(pathStr, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", pathStr.c_str()).c_str());
		return SQRESULT_ERROR;
	}
	try
	{
		sq_newarray(sqvm, 0);
		for (const auto& entry : fs::directory_iterator(path))
		{
			sq_pushstring(sqvm, entry.path().filename().string().c_str(), -1);
			sq_arrayappend(sqvm, -2);
		}
		return SQRESULT_NOTNULL;
	}
	catch (std::exception ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "DIR ITERATE FAILED! Is the path valid?\n");
		sq_raiseerror(sqvm, ex.what());
		return SQRESULT_ERROR;
	}
}

SQRESULT Script_NSIsFolder(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);
	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	std::string pathStr = sq_getstring(sqvm, 1);
	fs::path path = dir;
	if (pathStr != "")
		path = dir / pathStr;
	if (!IsPathSafe(pathStr, dir))
	{
		sq_raiseerror(sqvm, FormatA("File name invalid (%s)! Make sure it does not contain any non-ASCII character, and results in a path inside your mod's save folder.", pathStr.c_str()).c_str());
		return SQRESULT_ERROR;
	}
	try
	{
		sq_pushbool(sqvm, fs::is_directory(path));
		return SQRESULT_NOTNULL;
	}
	catch (std::exception ex)
	{
		Error(eLog::MODSYS, NO_ERROR, "DIR READ FAILED! Is the path valid?\n");
		DevMsg(eLog::MODSYS, "%s\n", path.string().c_str());
		sq_raiseerror(sqvm, ex.what());
		return SQRESULT_ERROR;
	}
}

// side note, expensive.
SQRESULT Script_NSGetTotalSpaceRemaining(HSQUIRRELVM sqvm)
{
	Mod* mod = sq_getcallingmod(sqvm);
	fs::path dir = g_svSavePath / fs::path(mod->m_ModDirectory).filename();
	sq_pushinteger(sqvm, (MAX_FOLDER_SIZE - GetSizeOfFolder(dir)) / 1024);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSGetModNames(HSQUIRRELVM sqvm)
{
	sq_newarray(sqvm, 0);

	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		sq_pushstring(sqvm, mod.Name.c_str(), -1);
		sq_arrayappend(sqvm, -2);
	}

	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSIsModEnabled(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			sq_pushbool(sqvm, mod.m_bEnabled);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSSetModEnabled(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);
	const SQBool enabled = sq_getbool(sqvm, 2);

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

SQRESULT Script_NSGetModDescriptionByModName(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			sq_pushstring(sqvm, mod.Description.c_str(), -1);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSGetModVersionByModName(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			sq_pushstring(sqvm, mod.Version.c_str(), -1);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSGetModDownloadLinkByModName(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			sq_pushstring(sqvm, mod.DownloadLink.c_str(), -1);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSGetModLoadPriority(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			sq_pushinteger(sqvm, mod.LoadPriority);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSIsModRequiredOnClient(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			sq_pushbool(sqvm, mod.RequiredOnClient);
			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSGetModConvarsByModName(HSQUIRRELVM sqvm)
{
	const SQChar* modName = sq_getstring(sqvm, 1);
	sq_newarray(sqvm, 0);

	// manual lookup, not super performant but eh not a big deal
	for (Mod& mod : g_pModManager->m_LoadedMods)
	{
		if (!mod.Name.compare(modName))
		{
			for (ModConVar* cvar : mod.ConVars)
			{
				sq_pushstring(sqvm, cvar->Name.c_str(), -1);
				sq_arrayappend(sqvm, -2);
			}

			return SQRESULT_NOTNULL;
		}
	}

	return SQRESULT_NOTNULL; // return empty array
}

SQRESULT Script_NSReloadMods(HSQUIRRELVM sqvm)
{
	NOTE_UNUSED(sqvm);
	g_pModManager->LoadMods();
	return SQRESULT_NULL;
}

SQRESULT Script_DecodeJSON(HSQUIRRELVM sqvm)
{
	const char* pJson = sq_getstring(sqvm, 1);
	const bool bFatalParseErrors = sq_getbool(sqvm, 2);

	nlohmann::json jsObj;

	try
	{
		jsObj = nlohmann::json::parse(pJson);
	}
	catch (const std::exception& ex)
	{
		sq_newtable(sqvm);

		std::string svError = FormatA("Failed parsing json file: encountered parse error: %s", ex.what());

		if (bFatalParseErrors)
		{
			sq_raiseerror(sqvm, svError.c_str());
			return SQRESULT_ERROR;
		}

		Warning(eLog::NS, "%s\n", svError.c_str());
		return SQRESULT_NOTNULL;
	}

	CScriptJson::DecodeJson(sqvm, jsObj);

	return SQRESULT_NOTNULL;
}

SQRESULT Script_EncodeJSON(HSQUIRRELVM sqvm)
{
	nlohmann::json jsObj;
	CScriptJson::EncodeJsonTable(sqvm->_stackOfCurrentFunction[1]._VAL.asTable, jsObj);

	std::string svObj = jsObj.dump();
	sq_pushstring(sqvm, svObj.c_str(), -1);
	return SQRESULT_NOTNULL;
}

// asset function StringToAsset( string assetName )
SQRESULT Script_StringToAsset(HSQUIRRELVM sqvm)
{
	sq_pushasset(sqvm, sq_getstring(sqvm, 1), -1);
	return SQRESULT_NOTNULL;
}

// string function NSGetLocalPlayerUID()
SQRESULT Script_NSGetLocalPlayerUID(HSQUIRRELVM sqvm)
{
	if (g_pLocalPlayerUserID)
	{
		sq_pushstring(sqvm, g_pLocalPlayerUserID, -1);
		return SQRESULT_NOTNULL;
	}

	return SQRESULT_NULL;
}

SQRESULT Script_NSGetCurrentModName(HSQUIRRELVM sqvm)
{
	int depth = sq_getinteger(sqvm, 1);
	if (auto mod = sq_getcallingmod(sqvm, depth); mod == nullptr)
	{
		sq_raiseerror(sqvm, "NSGetModName was called from a non-mod script. This shouldn't be possible");
		return SQRESULT_ERROR;
	}
	else
	{
		sq_pushstring(sqvm, mod->Name.c_str(), -1);
	}
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSGetCallingModName(HSQUIRRELVM sqvm)
{
	int depth = sq_getinteger(sqvm, 1);
	if (Mod* mod = sq_getcallingmod(sqvm, depth); mod == nullptr)
	{
		sq_pushstring(sqvm, "Unknown", -1);
	}
	else
	{
		sq_pushstring(sqvm, mod->Name.c_str(), -1);
	}
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NS_InternalMakeHttpRequest(HSQUIRRELVM sqvm)
{
	if (!g_pScriptHttp)
	{
		Warning(eLog::NS, "NS_InternalMakeHttpRequest called while the http request handler isn't running.\n");
		sq_pushinteger(sqvm, -1);
		return SQRESULT_NOTNULL;
	}

	if (g_pScriptHttp->IsHttpDisabled())
	{
		Warning(eLog::NS, "NS_InternalMakeHttpRequest called while the game is running with -disablehttprequests."
						  " Please check if requests are allowed using NSIsHttpEnabled() first.\n");
		sq_pushinteger(sqvm, -1);
		return SQRESULT_NOTNULL;
	}

	CScriptHttp::HttpRequest_t request;
	request.eMethod = static_cast<CScriptHttp::Type>(sq_getinteger(sqvm, 1));
	request.svBaseUrl = sq_getstring(sqvm, 2);

	// Read the tables for headers and query parameters.
	SQTable* headerTable = sqvm->_stackOfCurrentFunction[3]._VAL.asTable;
	for (int idx = 0; idx < headerTable->_numOfNodes; ++idx)
	{
		SQTable::_HashNode* node = &headerTable->_nodes[idx];

		if (node->key._Type == OT_STRING && node->val._Type == OT_ARRAY)
		{
			SQArray* valueArray = node->val._VAL.asArray;
			std::vector<std::string> headerValues;

			for (int vIdx = 0; vIdx < valueArray->_usedSlots; ++vIdx)
			{
				if (valueArray->_values[vIdx]._Type == OT_STRING)
				{
					headerValues.push_back(valueArray->_values[vIdx]._VAL.asString->_val);
				}
			}

			request.mpHeaders[node->key._VAL.asString->_val] = headerValues;
		}
	}

	SQTable* queryTable = sqvm->_stackOfCurrentFunction[4]._VAL.asTable;
	for (int idx = 0; idx < queryTable->_numOfNodes; ++idx)
	{
		SQTable::_HashNode* node = &queryTable->_nodes[idx];

		if (node->key._Type == OT_STRING && node->val._Type == OT_ARRAY)
		{
			SQArray* valueArray = node->val._VAL.asArray;
			std::vector<std::string> queryValues;

			for (int vIdx = 0; vIdx < valueArray->_usedSlots; ++vIdx)
			{
				if (valueArray->_values[vIdx]._Type == OT_STRING)
				{
					queryValues.push_back(valueArray->_values[vIdx]._VAL.asString->_val);
				}
			}

			request.mpQueryParameters[node->key._VAL.asString->_val] = queryValues;
		}
	}

	request.svContentType = sq_getstring(sqvm, 5);
	request.svBody = sq_getstring(sqvm, 6);
	request.nTimeout = sq_getinteger(sqvm, 7);
	request.svUserAgent = sq_getstring(sqvm, 8);

	int handle = g_pScriptHttp->MakeHttpRequest(request, (ScriptContext)sqvm->sharedState->cSquirrelVM->vmContext);
	sq_pushinteger(sqvm, handle);
	return SQRESULT_NOTNULL;
}

SQRESULT Script_NSIsHttpEnabled(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, !g_pScriptHttp->IsHttpDisabled());
	return SQRESULT_NULL;
}

SQRESULT Script_NSIsLocalHttpAllowed(HSQUIRRELVM sqvm)
{
	sq_pushbool(sqvm, g_pScriptHttp->IsLocalHttpAllowed());
	return SQRESULT_NULL;
}

//-----------------------------------------------------------------------------
// Purpose: Decodes a json object into a sq table
// Input  : *sqvm -
//          &jsObj -
//-----------------------------------------------------------------------------
void CScriptJson::DecodeJson(HSQUIRRELVM sqvm, nlohmann::json& jsObj)
{
	// FIXME [Fifty]: This is stupid, make the sq func return a var so we dont have to do this
	if (jsObj.type() == nlohmann::json::value_t::array)
	{
		sq_newtable(sqvm);
		sq_pushstring(sqvm, "RootArray", -1);
		DecodeArray(sqvm, jsObj);
		sq_newslot(sqvm, -3, false);
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
void CScriptJson::DecodeTable(HSQUIRRELVM sqvm, nlohmann::json& jsObj)
{
	sq_newtable(sqvm);

	for (auto& js : jsObj.items())
	{
		switch (js.value().type())
		{
		case nlohmann::json::value_t::boolean:
			sq_pushstring(sqvm, js.key().c_str(), -1);
			sq_pushbool(sqvm, js.value().get<bool>());
			sq_newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::string:
			sq_pushstring(sqvm, js.key().c_str(), -1);
			sq_pushstring(sqvm, js.value().get<std::string>().c_str(), -1);
			sq_newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::number_integer:
		case nlohmann::json::value_t::number_unsigned:
			sq_pushstring(sqvm, js.key().c_str(), -1);
			sq_pushinteger(sqvm, js.value().get<int>());
			sq_newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::number_float:
			sq_pushstring(sqvm, js.key().c_str(), -1);
			sq_pushfloat(sqvm, js.value().get<float>());
			sq_newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::object:
			sq_pushstring(sqvm, js.key().c_str(), -1);
			DecodeTable(sqvm, js.value());
			sq_newslot(sqvm, -3, false);
			break;
		case nlohmann::json::value_t::array:
			sq_pushstring(sqvm, js.key().c_str(), -1);
			DecodeArray(sqvm, js.value());
			sq_newslot(sqvm, -3, false);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Decodes a json object into a sq array
// Input  : *sqvm -
//          &jsObj -
//-----------------------------------------------------------------------------
void CScriptJson::DecodeArray(HSQUIRRELVM sqvm, nlohmann::json& jsObj)
{
	sq_newarray(sqvm, 0);

	for (auto& js : jsObj)
	{
		switch (js.type())
		{
		case nlohmann::json::value_t::boolean:
			sq_pushbool(sqvm, js.get<bool>());
			sq_arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::string:
			sq_pushstring(sqvm, js.get<std::string>().c_str(), -1);
			sq_arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::number_integer:
		case nlohmann::json::value_t::number_unsigned:
			sq_pushinteger(sqvm, js.get<int>());
			sq_arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::number_float:
			sq_pushfloat(sqvm, js.get<float>());
			sq_arrayappend(sqvm, -2);
			break;
		case nlohmann::json::value_t::object:
			DecodeTable(sqvm, js);
			sq_arrayappend(sqvm, -2);
			break;
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose: Encodes json represented as a sq table into a json obj
// Input  : *pTable -
//          &jsObj -
//-----------------------------------------------------------------------------
void CScriptJson::EncodeJsonTable(SQTable* pTable, nlohmann::json& jsObj)
{
	for (int i = 0; i < pTable->_numOfNodes; i++)
	{
		SQTable::_HashNode* node = &pTable->_nodes[i];

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
				Warning(eLog::NS, "CScriptJson::EncodeJsonTable: squirrel type %s not supported\n", SQ_GetTypeAsString(node->val._Type));
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
void CScriptJson::EncodeJsonArray(SQArray* pArray, nlohmann::json& jsObj)
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
			Warning(eLog::NS, "CScriptJson::EncodeJsonArray: squirrel type %s not supported\n", SQ_GetTypeAsString(node->_Type));
		}
	}
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
CScriptHttp::CScriptHttp()
{
	m_bIsHttpDisabled = CommandLine()->FindParm("-disablehttprequests");
	m_bIsLocalHttpAllowed = CommandLine()->FindParm("-allowlocalhttp");
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CScriptHttp::IsHttpDestinationHostAllowed(const std::string& host, std::string& outHostname, std::string& outAddress, std::string& outPort)
{
	CURLU* url = curl_url();
	if (!url)
	{
		Error(eLog::NS, NO_ERROR, "Failed to call curl_url() for http request.\n");
		return false;
	}

	if (curl_url_set(url, CURLUPART_URL, host.c_str(), CURLU_DEFAULT_SCHEME) != CURLUE_OK)
	{
		Error(eLog::NS, NO_ERROR, "Failed to parse destination URL for http request.\n");

		curl_url_cleanup(url);
		return false;
	}

	char* urlHostname = nullptr;
	if (curl_url_get(url, CURLUPART_HOST, &urlHostname, 0) != CURLUE_OK)
	{
		Error(eLog::NS, NO_ERROR, "Failed to parse hostname from destination URL for http request.\n");

		curl_url_cleanup(url);
		return false;
	}

	char* urlScheme = nullptr;
	if (curl_url_get(url, CURLUPART_SCHEME, &urlScheme, CURLU_DEFAULT_SCHEME) != CURLUE_OK)
	{
		Error(eLog::NS, NO_ERROR, "Failed to parse scheme from destination URL for http request.\n");

		curl_url_cleanup(url);
		curl_free(urlHostname);
		return false;
	}

	char* urlPort = nullptr;
	if (curl_url_get(url, CURLUPART_PORT, &urlPort, CURLU_DEFAULT_PORT) != CURLUE_OK)
	{
		Error(eLog::NS, NO_ERROR, "Failed to parse port from destination URL for http request.\n");

		curl_url_cleanup(url);
		curl_free(urlHostname);
		curl_free(urlScheme);
		return false;
	}

	// Resolve the hostname into an address.
	addrinfo* result;
	addrinfo hints;
	std::memset(&hints, 0, sizeof(addrinfo));
	hints.ai_family = AF_UNSPEC;

	if (getaddrinfo(urlHostname, urlScheme, &hints, &result) != 0)
	{
		Error(eLog::NS, NO_ERROR, "Failed to resolve http request destination %s using getaddrinfo().\n", urlHostname);

		curl_url_cleanup(url);
		curl_free(urlHostname);
		curl_free(urlScheme);
		curl_free(urlPort);
		return false;
	}

	bool bFoundIPv6 = false;
	sockaddr_in* sockaddr_ipv4 = nullptr;
	for (addrinfo* info = result; info; info = info->ai_next)
	{
		if (info->ai_family == AF_INET)
		{
			sockaddr_ipv4 = (sockaddr_in*)info->ai_addr;
			break;
		}

		bFoundIPv6 = bFoundIPv6 || info->ai_family == AF_INET6;
	}

	if (sockaddr_ipv4 == nullptr)
	{
		if (bFoundIPv6)
		{
			Error(eLog::NS, NO_ERROR, "Only IPv4 destinations are supported for HTTP requests. To allow IPv6, launch the game using -allowlocalhttp.\n");
		}
		else
		{
			Error(eLog::NS, NO_ERROR, "Failed to resolve http request destination %s into a valid IPv4 address.\n", urlHostname);
		}

		curl_free(urlHostname);
		curl_free(urlScheme);
		curl_free(urlPort);
		curl_url_cleanup(url);

		return false;
	}

	// Fast checks for private ranges of IPv4.
	// clang-format off
	{
		auto addrBytes = sockaddr_ipv4->sin_addr.S_un.S_un_b;

		if (addrBytes.s_b1 == 10														// 10.0.0.0			- 10.255.255.255		(Class A Private)
			|| addrBytes.s_b1 == 172 && addrBytes.s_b2 >= 16 && addrBytes.s_b2 <= 31	// 172.16.0.0		- 172.31.255.255		(Class B Private)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 168							// 192.168.0.0		- 192.168.255.255		(Class C Private)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 0 && addrBytes.s_b3 == 0		// 192.0.0.0		- 192.0.0.255			(IETF Assignment)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 0 && addrBytes.s_b3 == 2		// 192.0.2.0		- 192.0.2.255			(TEST-NET-1)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 88 && addrBytes.s_b3 == 99	// 192.88.99.0		- 192.88.99.255			(IPv4-IPv6 Relay)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 >= 18 &&	addrBytes.s_b2 <= 19	// 192.18.0.0		- 192.19.255.255		(Internet Benchmark)
			|| addrBytes.s_b1 == 192 && addrBytes.s_b2 == 51 && addrBytes.s_b3 == 100	// 192.51.100.0		- 192.51.100.255		(TEST-NET-2)
			|| addrBytes.s_b1 == 203 && addrBytes.s_b2 == 0 && addrBytes.s_b3 == 113	// 203.0.113.0		- 203.0.113.255			(TEST-NET-3)
			|| addrBytes.s_b1 == 169 && addrBytes.s_b2 == 254							// 169.254.00		- 169.254.255.255		(Link-local/APIPA)
			|| addrBytes.s_b1 == 127													// 127.0.0.0		- 127.255.255.255		(Loopback)
			|| addrBytes.s_b1 == 0														// 0.0.0.0			- 0.255.255.255			(Current network)
			|| addrBytes.s_b1 == 100 && addrBytes.s_b2 >= 64 && addrBytes.s_b2 <= 127	// 100.64.0.0		- 100.127.255.255		(Shared address space)
			|| sockaddr_ipv4->sin_addr.S_un.S_addr == 0xFFFFFFFF						// 255.255.255.255							(Broadcast)
			|| addrBytes.s_b1 >= 224 && addrBytes.s_b2 <= 239							// 224.0.0.0		- 239.255.255.255		(Multicast)
			|| addrBytes.s_b1 == 233 && addrBytes.s_b2 == 252 && addrBytes.s_b3 == 0	// 233.252.0.0		- 233.252.0.255			(MCAST-TEST-NET)
			|| addrBytes.s_b1 >= 240 && addrBytes.s_b4 <= 254)							// 240.0.0.0		- 255.255.255.254		(Future Use Class E)
		{
			curl_free(urlHostname);
			curl_free(urlScheme);
			curl_free(urlPort);
			curl_url_cleanup(url);

			return false;
		}
	}

	// clang-format on

	char resolvedStr[INET_ADDRSTRLEN];
	inet_ntop(AF_INET, &sockaddr_ipv4->sin_addr, resolvedStr, INET_ADDRSTRLEN);

	// Use the resolved address as the new request host.
	outHostname = urlHostname;
	outAddress = resolvedStr;
	outPort = urlPort;

	freeaddrinfo(result);

	curl_free(urlHostname);
	curl_free(urlScheme);
	curl_free(urlPort);
	curl_url_cleanup(url);

	return true;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
int CScriptHttp::MakeHttpRequest(const HttpRequest_t& requestParameters, ScriptContext nContext)
{
	if (IsHttpDisabled())
	{
		Warning(eLog::NS, "NS_InternalMakeHttpRequest called while the game is running with -disablehttprequests."
						  " Please check if requests are allowed using NSIsHttpEnabled() first.\n");
		return -1;
	}

	bool bAllowLocalHttp = IsLocalHttpAllowed();

	// This handle will be returned to Squirrel so it can wait for the response and assign a callback for it.
	int handle = ++m_iLastRequestHandle;

	std::thread(
		[this, handle, requestParameters, bAllowLocalHttp, nContext]()
		{
			std::string hostname, resolvedAddress, resolvedPort;

			if (!bAllowLocalHttp)
			{
				if (!IsHttpDestinationHostAllowed(requestParameters.svBaseUrl, hostname, resolvedAddress, resolvedPort))
				{
					Warning(eLog::NS, "HttpRequestHandler::MakeHttpRequest attempted to make a request to a private network. This is only allowed when "
									  "running the game with -allowlocalhttp.\n");
					g_pTaskScheduler->AddTask(
						[handle, nContext]()
						{
							CSquirrelVM* pVM = nullptr;
							if (nContext == ScriptContext::SERVER)
								pVM = g_pServerVM;
							else if (nContext == ScriptContext::CLIENT)
								pVM = g_pClientVM;
							else if (nContext == ScriptContext::UI)
								pVM = g_pUIVM;

							if (pVM && pVM->GetVM())
							{
								ScriptContext nContext = (ScriptContext)pVM->vmContext;
								HSQUIRRELVM hVM = pVM->GetVM();
								const char* pszFunction = "NSHandleFailedHttpRequest";

								SQObject oFunction {};
								int nResult = sq_getfunction(hVM, pszFunction, &oFunction, 0);
								if (nResult != 0)
								{
									Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFunction);
									return;
								}

								// Push
								sq_pushobject(hVM, &oFunction);
								sq_pushroottable(hVM);

								sq_pushinteger(hVM, handle);
								sq_pushinteger(hVM, 0);
								sq_pushstring(hVM, "Cannot make HTTP requests to private network hosts without -allowlocalhttp. Check your console for more information.", -1);

								(void)sq_call(hVM, 4, false, false);
							}
						});
					// g_pSquirrel<context>->AsyncCall("NSHandleFailedHttpRequest", handle, (int)0,
					//								"Cannot make HTTP requests to private network hosts without -allowlocalhttp. Check your console for more "
					//								"information.");
					return;
				}
			}

			CURL* curl = curl_easy_init();
			if (!curl)
			{
				Error(eLog::NS, NO_ERROR, "HttpRequestHandler::MakeHttpRequest failed to init libcurl for request.\n");
				std::string svError = curl_easy_strerror(CURLE_FAILED_INIT);
				g_pTaskScheduler->AddTask(
					[handle, svError, nContext]()
					{
						CSquirrelVM* pVM = nullptr;
						if (nContext == ScriptContext::SERVER)
							pVM = g_pServerVM;
						else if (nContext == ScriptContext::CLIENT)
							pVM = g_pClientVM;
						else if (nContext == ScriptContext::UI)
							pVM = g_pUIVM;

						if (pVM && pVM->GetVM())
						{
							ScriptContext nContext = (ScriptContext)pVM->vmContext;
							HSQUIRRELVM hVM = pVM->GetVM();
							const char* pszFunction = "NSHandleFailedHttpRequest";

							SQObject oFunction {};
							int nResult = sq_getfunction(hVM, pszFunction, &oFunction, 0);
							if (nResult != 0)
							{
								Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFunction);
								return;
							}

							// Push
							sq_pushobject(hVM, &oFunction);
							sq_pushroottable(hVM);

							sq_pushinteger(hVM, handle);
							sq_pushinteger(hVM, static_cast<int>(CURLE_FAILED_INIT));
							sq_pushstring(hVM, svError.c_str(), -1);

							(void)sq_call(hVM, 4, false, false);
						}
					});
				// g_pSquirrel<context>->AsyncCall("NSHandleFailedHttpRequest", handle, static_cast<int>(CURLE_FAILED_INIT), curl_easy_strerror(CURLE_FAILED_INIT));
				return;
			}

			// HEAD has no body.
			if (requestParameters.eMethod == HTTP_HEAD)
			{
				curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
			}

			curl_easy_setopt(curl, CURLOPT_CUSTOMREQUEST, ToString(requestParameters.eMethod).c_str());

			// Only resolve to IPv4 if we don't allow private network requests.
			curl_slist* host = nullptr;
			if (!bAllowLocalHttp)
			{
				curl_easy_setopt(curl, CURLOPT_IPRESOLVE, CURL_IPRESOLVE_V4);
				host = curl_slist_append(host, FormatA("%s:%s:%s", hostname.c_str(), resolvedPort.c_str(), resolvedAddress.c_str()).c_str());
				curl_easy_setopt(curl, CURLOPT_RESOLVE, host);
			}

			// Ensure we only allow HTTP or HTTPS.
			curl_easy_setopt(curl, CURLOPT_PROTOCOLS, CURLPROTO_HTTP | CURLPROTO_HTTPS);

			// Allow redirects
			curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
			curl_easy_setopt(curl, CURLOPT_MAXREDIRS, 3L);

			// Check if the url already contains a query.
			// If so, we'll know to append with & instead of start with ?
			std::string queryUrl = requestParameters.svBaseUrl;
			bool bUrlContainsQuery = false;

			// If this fails, just ignore the parsing and trust what the user wants to query.
			// Probably will fail but handling it here would be annoying.
			CURLU* curlUrl = curl_url();
			if (curlUrl)
			{
				if (curl_url_set(curlUrl, CURLUPART_URL, queryUrl.c_str(), CURLU_DEFAULT_SCHEME) == CURLUE_OK)
				{
					char* currentQuery;
					if (curl_url_get(curlUrl, CURLUPART_QUERY, &currentQuery, 0) == CURLUE_OK)
					{
						if (currentQuery && std::strlen(currentQuery) != 0)
						{
							bUrlContainsQuery = true;
						}
					}

					curl_free(currentQuery);
				}

				curl_url_cleanup(curlUrl);
			}

			// GET requests, or POST-like requests with an empty body, can have query parameters.
			// Append them to the base url.
			if (CanHaveQueryParameters(requestParameters.eMethod) && !UsesCurlPostOptions(requestParameters.eMethod) || requestParameters.svBody.empty())
			{
				bool isFirstValue = true;
				for (const auto& kv : requestParameters.mpQueryParameters)
				{
					char* key = curl_easy_escape(curl, kv.first.c_str(), kv.first.length());

					for (const std::string& queryValue : kv.second)
					{
						char* value = curl_easy_escape(curl, queryValue.c_str(), queryValue.length());

						if (isFirstValue && !bUrlContainsQuery)
						{
							queryUrl.append(FormatA("?%s=%s", key, value));
							isFirstValue = false;
						}
						else
						{
							queryUrl.append(FormatA("&%s=%s", key, value));
						}

						curl_free(value);
					}

					curl_free(key);
				}
			}

			// If this method uses POST-like curl options, set those and set the body.
			// The body won't be sent if it's empty anyway, meaning the query parameters above, if any, would be.
			if (UsesCurlPostOptions(requestParameters.eMethod))
			{
				// Grab the body and set it as a POST field
				curl_easy_setopt(curl, CURLOPT_POST, 1L);

				curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, requestParameters.svBody.length());
				curl_easy_setopt(curl, CURLOPT_COPYPOSTFIELDS, requestParameters.svBody.c_str());
			}

			// Set the full URL for this http request.
			curl_easy_setopt(curl, CURLOPT_URL, queryUrl.c_str());

			std::string bodyBuffer;
			std::string headerBuffer;

			// Set up buffers to write the response headers and body.
			curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, CURLWriteStringCallback);
			curl_easy_setopt(curl, CURLOPT_WRITEDATA, &bodyBuffer);
			curl_easy_setopt(curl, CURLOPT_HEADERFUNCTION, CURLWriteStringCallback);
			curl_easy_setopt(curl, CURLOPT_HEADERDATA, &headerBuffer);

			// Add all the headers for the request.
			curl_slist* headers = nullptr;

			// Content-Type header for POST-like requests.
			if (UsesCurlPostOptions(requestParameters.eMethod) && !requestParameters.svBody.empty())
			{
				headers = curl_slist_append(headers, FormatA("Content-Type: %s", requestParameters.svContentType.c_str()).c_str());
			}

			for (const auto& kv : requestParameters.mpHeaders)
			{
				for (const std::string& headerValue : kv.second)
				{
					headers = curl_slist_append(headers, FormatA("%s: %s", kv.first.c_str(), headerValue.c_str()).c_str());
				}
			}

			if (headers != nullptr)
			{
				curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
			}

			// Disable SSL checks if requested by the user.
			if (false) // TODO: make this a cvar
			{
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
				curl_easy_setopt(curl, CURLOPT_SSL_VERIFYSTATUS, 0L);
			}

			// Enforce the Northstar user agent, unless an override was specified.
			if (requestParameters.svUserAgent.empty())
			{
				curl_easy_setopt(curl, CURLOPT_USERAGENT, NORTHSTAR_USERAGENT);
			}
			else
			{
				curl_easy_setopt(curl, CURLOPT_USERAGENT, requestParameters.svUserAgent.c_str());
			}

			// Set the timeout for this request. Max 60 seconds so mods can't just spin up native threads all the time.
			curl_easy_setopt(curl, CURLOPT_TIMEOUT, std::clamp<long>(requestParameters.nTimeout, 1, 60));

			CURLcode result = curl_easy_perform(curl);
			if (result == CURLE_OK)
			{
				// While the curl request is OK, it could return a non success code.
				// Squirrel side will handle firing the correct callback.
				long httpCode = 0;
				curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
				g_pTaskScheduler->AddTask(
					[handle, httpCode, bodyBuffer, headerBuffer, nContext]()
					{
						CSquirrelVM* pVM = nullptr;
						if (nContext == ScriptContext::SERVER)
							pVM = g_pServerVM;
						else if (nContext == ScriptContext::CLIENT)
							pVM = g_pClientVM;
						else if (nContext == ScriptContext::UI)
							pVM = g_pUIVM;

						if (pVM && pVM->GetVM())
						{
							ScriptContext nContext = (ScriptContext)pVM->vmContext;
							HSQUIRRELVM hVM = pVM->GetVM();
							const char* pszFunction = "NSHandleSuccessfulHttpRequest";

							SQObject oFunction {};
							int nResult = sq_getfunction(hVM, pszFunction, &oFunction, 0);
							if (nResult != 0)
							{
								Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFunction);
								return;
							}

							// Push
							sq_pushobject(hVM, &oFunction);
							sq_pushroottable(hVM);

							sq_pushinteger(hVM, handle);
							sq_pushinteger(hVM, static_cast<int>(httpCode));
							sq_pushstring(hVM, bodyBuffer.c_str(), -1);
							sq_pushstring(hVM, headerBuffer.c_str(), -1);

							(void)sq_call(hVM, 5, false, false);
						}
					});
				// g_pSquirrel<context>->AsyncCall("NSHandleSuccessfulHttpRequest", handle, static_cast<int>(httpCode), bodyBuffer, headerBuffer);
			}
			else
			{
				// Pass CURL result code & error.
				Error(eLog::NS, NO_ERROR, "curl_easy_perform() failed with code %i, error: %s\n", static_cast<int>(result), curl_easy_strerror(result));

				// If it's an SSL issue, tell the user they may disable SSL checks using -disablehttpssl.
				if (result == CURLE_PEER_FAILED_VERIFICATION || result == CURLE_SSL_CERTPROBLEM || result == CURLE_SSL_INVALIDCERTSTATUS)
				{
					Warning(eLog::NS, "You can try disabling SSL verifications for this issue using the -disablehttpssl launch argument. "
									  "Keep in mind this is potentially dangerous!\n");
				}
				std::string svError = curl_easy_strerror(result);
				g_pTaskScheduler->AddTask(
					[handle, result, svError, nContext]()
					{
						CSquirrelVM* pVM = nullptr;
						if (nContext == ScriptContext::SERVER)
							pVM = g_pServerVM;
						else if (nContext == ScriptContext::CLIENT)
							pVM = g_pClientVM;
						else if (nContext == ScriptContext::UI)
							pVM = g_pUIVM;

						if (pVM && pVM->GetVM())
						{
							ScriptContext nContext = (ScriptContext)pVM->vmContext;
							HSQUIRRELVM hVM = pVM->GetVM();
							const char* pszFunction = "NSHandleFailedHttpRequest";

							SQObject oFunction {};
							int nResult = sq_getfunction(hVM, pszFunction, &oFunction, 0);
							if (nResult != 0)
							{
								Error(VScript_GetNativeLogContext(nContext), NO_ERROR, "Call was unable to find function with name '%s'. Is it global?\n", pszFunction);
								return;
							}

							// Push
							sq_pushobject(hVM, &oFunction);
							sq_pushroottable(hVM);

							sq_pushinteger(hVM, handle);
							sq_pushinteger(hVM, static_cast<int>(result));
							sq_pushstring(hVM, svError.c_str(), -1);

							(void)sq_call(hVM, 4, false, false);
						}
					});
				// g_pSquirrel<context>->AsyncCall("NSHandleFailedHttpRequest", handle, static_cast<int>(result), curl_easy_strerror(result));
			}

			curl_easy_cleanup(curl);
			curl_slist_free_all(headers);
			curl_slist_free_all(host);
		})
		.detach();
	return handle;
}

void VScript_RegisterSharedFunctions(CSquirrelVM* vm)
{
	vm->RegisterFunction("NSSaveFile", "Script_NSSaveFile", "", "void", "string file, string data", Script_NSSaveFile);
	vm->RegisterFunction("NSSaveJSONFile", "Script_NSSaveJSONFile", "", "void", "string file, table data", Script_NSSaveJSONFile);
	vm->RegisterFunction("NS_InternalLoadFile", "Script_NS_InternalLoadFile", "", "int", "string file", Script_NS_InternalLoadFile);
	vm->RegisterFunction("NSDoesFileExist", "Script_NSDoesFileExist", "", "bool", "string file", Script_NSDoesFileExist);
	vm->RegisterFunction("NSGetFileSize", "Script_NSGetFileSize", "", "int", "string file", Script_NSGetFileSize);
	vm->RegisterFunction("NSDeleteFile", "Script_NSDeleteFile", "", "void", "string file", Script_NSDeleteFile);
	vm->RegisterFunction("NS_InternalGetAllFiles", "Script_NS_InternalGetAllFiles", "", "array<string>", "string path", Script_NS_InternalGetAllFiles);
	vm->RegisterFunction("NSIsFolder", "Script_NSIsFolder", "", "bool", "string path", Script_NSIsFolder);
	vm->RegisterFunction("NSGetTotalSpaceRemaining", "Script_NSGetTotalSpaceRemaining", "", "int", "", Script_NSGetTotalSpaceRemaining);
	vm->RegisterFunction("NSGetModNames", "Script_NSGetModNames", "", "array<string>", "", Script_NSGetModNames);
	vm->RegisterFunction("NSIsModEnabled", "Script_NSIsModEnabled", "", "bool", "string modName", Script_NSIsModEnabled);
	vm->RegisterFunction("NSSetModEnabled", "Script_NSSetModEnabled", "", "void", "string modName, bool enabled", Script_NSSetModEnabled);
	vm->RegisterFunction("NSGetModDescriptionByModName", "Script_NSGetModDescriptionByModName", "", "string", "string modName", Script_NSGetModDescriptionByModName);
	vm->RegisterFunction("NSGetModVersionByModName", "Script_NSGetModVersionByModName", "", "string", "string modName", Script_NSGetModVersionByModName);
	vm->RegisterFunction("NSGetModDownloadLinkByModName", "Script_NSGetModDownloadLinkByModName", "", "string", "string modName", Script_NSGetModDownloadLinkByModName);
	vm->RegisterFunction("NSGetModLoadPriority", "Script_NSGetModLoadPriority", "", "int", "string modName", Script_NSGetModLoadPriority);
	vm->RegisterFunction("NSIsModRequiredOnClient", "Script_NSIsModRequiredOnClient", "", "bool", "string modName", Script_NSIsModRequiredOnClient);
	vm->RegisterFunction("NSGetModConvarsByModName", "Script_NSGetModConvarsByModName", "", "array<string>", "string modName", Script_NSGetModConvarsByModName);
	vm->RegisterFunction("NSReloadMods", "Script_NSReloadMods", "", "void", "", Script_NSReloadMods);
	vm->RegisterFunction("DecodeJSON", "Script_DecodeJSON", "", "table", "string json, bool fatalParseErrors = false", Script_DecodeJSON);
	vm->RegisterFunction("EncodeJSON", "Script_EncodeJSON", "", "string", "table data", Script_EncodeJSON);
	vm->RegisterFunction("StringToAsset", "Script_StringToAsset", "", "asset", "string assetName", Script_StringToAsset);
	vm->RegisterFunction("NSGetLocalPlayerUID", "Script_NSGetLocalPlayerUID", "", "string", "", Script_NSGetLocalPlayerUID);
	vm->RegisterFunction("NSGetCurrentModName", "Script_NSGetCurrentModName", "", "string", "", Script_NSGetCurrentModName);
	vm->RegisterFunction("NSGetCallingModName", "Script_NSGetCallingModName", "", "string", "int depth = 0", Script_NSGetCallingModName);
	vm->RegisterFunction("NS_InternalMakeHttpRequest", "Script_NS_InternalMakeHttpRequest", "", "int",
						 "int method, string baseUrl, table<string, array<string> > headers, table<string, array<string> > queryParams, string contentType, string body, int timeout, string userAgent", Script_NS_InternalMakeHttpRequest);
	vm->RegisterFunction("NSIsHttpEnabled", "Script_NSIsHttpEnabled", "", "bool", "", Script_NSIsHttpEnabled);
	vm->RegisterFunction("NSIsLocalHttpAllowed", "Script_NSIsLocalHttpAllowed", "", "bool", "", Script_NSIsLocalHttpAllowed);
}
