#include "filesystem.h"

std::error_code fsErrorCode;

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
std::error_code FSGetLastErrorCode() noexcept
{
	return fsErrorCode;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool FileExists(fs::path path) noexcept
{
	std::error_code errorCode;
	return fs::exists(path, errorCode);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool IsDirectory(fs::path path) noexcept
{
	std::error_code errorCode;
	return fs::is_directory(path, errorCode);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CopyFile(fs::path from, fs::path to) noexcept
{
	std::error_code errorCode;
	fs::copy_file(from, to, errorCode);

	if (errorCode.value() == 0)
		return true;

	fsErrorCode = errorCode;
	return false;
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CreateDirectories(fs::path path) noexcept
{
	std::error_code errorCode;
	fs::create_directories(path, errorCode);

	if (errorCode.value() == 0)
		return true;

	fsErrorCode = errorCode;
	return false;
}
