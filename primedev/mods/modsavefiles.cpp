#include "modsavefiles.h"

/// <summary></summary>
/// <param name="dir">The directory we want the size of.</param>
/// <param name="file">The file we're excluding from the calculation.</param>
/// <returns>The size of the contents of the current directory, excluding a specific file.</returns>
uintmax_t GetSizeOfFolderContentsMinusFile(fs::path dir, std::string file)
{
	uintmax_t result = 0;
	for (const auto& entry : fs::directory_iterator(dir))
	{
		if (entry.path().filename() == file)
			continue;
		// fs::file_size may not work on directories - but does in some cases.
		// per cppreference.com, it's "implementation-defined".
		try
		{
			result += fs::file_size(entry.path());
		}
		catch (fs::filesystem_error& e)
		{
			if (entry.is_directory())
			{
				result += GetSizeOfFolderContentsMinusFile(entry.path(), "");
			}
		}
	}
	return result;
}

uintmax_t GetSizeOfFolder(fs::path dir)
{
	uintmax_t result = 0;
	for (const auto& entry : fs::directory_iterator(dir))
	{
		// fs::file_size may not work on directories - but does in some cases.
		// per cppreference.com, it's "implementation-defined".
		try
		{
			result += fs::file_size(entry.path());
		}
		catch (fs::filesystem_error& e)
		{
			if (entry.is_directory())
			{
				result += GetSizeOfFolderContentsMinusFile(entry.path(), "");
			}
		}
	}
	return result;
}

// Checks if a file contains null characters.
bool ContainsInvalidChars(std::string str)
{
	// we don't allow null characters either, even if they're ASCII characters because idk if people can
	// use it to circumvent the file extension suffix.
	return std::any_of(str.begin(), str.end(), [](char c) { return c == '\0'; });
}

// Checks if the relative path (param) remains inside the mod directory (dir).
// Paths are restricted to ASCII because encoding is fucked and we decided we won't bother.
bool IsPathSafe(const std::string param, fs::path dir)
{
	try
	{
		auto const normRoot = fs::weakly_canonical(dir);
		auto const normChild = fs::weakly_canonical(dir / param);

		auto itr = std::search(normChild.begin(), normChild.end(), normRoot.begin(), normRoot.end());
		// we return if the file is safe (inside the directory) and uses only ASCII chars in the path.
		return itr == normChild.begin() && std::none_of(param.begin(), param.end(),
														[](char c)
														{
															unsigned char unsignedC = static_cast<unsigned char>(c);
															return unsignedC > 127 || unsignedC < 0;
														});
	}
	catch (fs::filesystem_error err)
	{
		return false;
	}
}

ON_DLL_LOAD("engine.dll", ModSaveFFiles_Init, (CModule module))
{
	g_svSavePath = fs::path(g_svProfileDir) / "save_data";
	g_pSaveFileManager = new SaveFileManager;
	int parm = CommandLine()->FindParm("-maxfoldersize");

	if (parm)
		MAX_FOLDER_SIZE = std::stoi(CommandLine()->GetParm(parm));
}

int GetMaxSaveFolderSize()
{
	return MAX_FOLDER_SIZE;
}
