#pragma once

#include <string>

void RemoveAsciiControlSequences(char* str, bool allow_color_codes);

std::string FormatAV(const char* fmt, va_list vArgs);
std::string FormatA(const char* fmt, ...);

std::wstring FormatWV(const wchar_t* fmt, va_list vArgs);
std::wstring FormatW(const wchar_t* fmt, ...);

std::string CreateTimeStamp();
