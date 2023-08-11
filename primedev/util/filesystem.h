#pragma once

#include <filesystem>

namespace fs = std::filesystem;

//-----------------------------------------------------------------------------
//
std::error_code FSGetLastErrorCode() noexcept;

//-----------------------------------------------------------------------------
//
bool FileExists(fs::path path) noexcept;
bool IsDirectory(fs::path path) noexcept;
bool CopyFile(fs::path from, fs::path to) noexcept;
bool CreateDirectories(fs::path path) noexcept;
