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
	return fs::exists(path, fsErrorCode);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool IsDirectory(fs::path path) noexcept
{
	return fs::is_directory(path, fsErrorCode);
}

//-----------------------------------------------------------------------------
// Purpose:
//-----------------------------------------------------------------------------
bool CopyFile(fs::path from, fs::path to) noexcept
{
	return fs::copy_file(from, to, fsErrorCode);
}
