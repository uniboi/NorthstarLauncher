#include <regex>

LANGID (*Tier0_DetectDefaultLanguage)();

char* g_pszInGameLang;
bool* g_bCanOriginDetectLang;

bool CheckLangAudioExists(char* pszLang)
{
	fs::path lang = FormatA("r2\\sound\\general_%s.mstr", pszLang);

	return FileExists(lang);
}

std::vector<std::string> file_list(fs::path dir, std::regex ext_pattern)
{
	std::vector<std::string> result;

	if (!FileExists(dir) || !fs::is_directory(dir))
		return result;

	using iterator = fs::directory_iterator;

	const iterator end;
	for (iterator iter {dir}; iter != end; ++iter)
	{
		const std::string filename = iter->path().filename().string();
		std::smatch matches;
		if (fs::is_regular_file(*iter) && std::regex_match(filename, matches, ext_pattern))
		{
			result.push_back(std::move(matches.str(1)));
		}
	}

	return result;
}

std::string GetAnyInstalledAudioLanguage()
{
	for (const auto& lang : file_list("r2\\sound\\", std::regex(".*?general_([a-z]+)_patch_1\\.mstr")))
		if (lang != "general" || lang != "")
			return lang;
	return "NO LANGUAGE DETECTED";
}

char* (*o_GetGameLanguage)();

// clang-format off
char* h_GetGameLanguage()
// clang-format on
{
	const char* forcedLanguage;
	if (CommandLine()->CheckParm("-language", &forcedLanguage))
	{
		if (!CheckLangAudioExists((char*)forcedLanguage))
		{
			Warning(eLog::AUDIO,
					"User tried to force the language (-language) to \"%s\", but audio for this language doesn't exist and the game is bound "
					"to error, falling back to next option...\n",
					forcedLanguage);
		}
		else
		{
			DevMsg(eLog::AUDIO, "User forcing the language (-language) to: %s\n", forcedLanguage);
			strncpy(g_pszInGameLang, forcedLanguage, 256);
			return g_pszInGameLang;
		}
	}

	*g_bCanOriginDetectLang = true; // let it try
	{
		auto lang = o_GetGameLanguage();
		if (!CheckLangAudioExists(lang))
		{
			if (strcmp(lang, "russian") != 0) // don't log for "russian" since it's the default and that means Origin detection just didn't change it most likely
				DevMsg(eLog::AUDIO, "Origin detected language \"%s\", but we do not have audio for it installed, falling back to the next option\n", lang);
		}
		else
		{
			DevMsg(eLog::AUDIO, "Origin detected language: %s\n", lang);
			return lang;
		}
	}

	Tier0_DetectDefaultLanguage(); // force the global in tier0 to be populated with language inferred from user's system rather than
									   // defaulting to Russian
	*g_bCanOriginDetectLang = false; // Origin has no say anymore, we will fallback to user's system setup language
	auto lang = o_GetGameLanguage();
	DevMsg(eLog::AUDIO, "Detected system language: %s\n", lang);
	if (!CheckLangAudioExists(lang))
	{
		Warning(eLog::AUDIO, "Caution, audio for this language does NOT exist. You might want to override your game language with -language command line "
							 "option.\n");
		auto lang = GetAnyInstalledAudioLanguage();
		Warning(eLog::AUDIO, "Falling back to the first installed audio language: %s\n", lang.c_str());
		strncpy(g_pszInGameLang, lang.c_str(), 256);
		return g_pszInGameLang;
	}

	return lang;
}

ON_DLL_LOAD_CLIENT("tier0.dll", LanguageHooks, (CModule module))
{
	o_GetGameLanguage = module.Offset(0xF560).RCast<char* (*)()>();
	HookAttach(&(PVOID&)o_GetGameLanguage, (PVOID)h_GetGameLanguage);

	g_pszInGameLang = module.Offset(0xA9B60).RCast<char*>(); // Size 256
	g_bCanOriginDetectLang = module.Offset(0xA9A90).RCast<bool*>();

	Tier0_DetectDefaultLanguage = module.GetExportedFunction("Tier0_DetectDefaultLanguage").RCast<LANGID (*)()>();
}
