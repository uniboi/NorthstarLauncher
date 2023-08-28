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

//-----------------------------------------------------------------------------
//
void RemoveAsciiControlSequences(char* str, bool allow_color_codes);

//-----------------------------------------------------------------------------
//
std::string FormatAV(const char* fmt, va_list vArgs);
std::string FormatA(const char* fmt, ...);

std::wstring FormatWV(const wchar_t* fmt, va_list vArgs);
std::wstring FormatW(const wchar_t* fmt, ...);

//-----------------------------------------------------------------------------
//
std::string CreateTimeStamp();
