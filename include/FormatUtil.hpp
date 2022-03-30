#pragma once
#include <string>

#include <windows.h>

namespace IngameIME
{
std::string  format(const HRESULT hr);
std::string  format(const char* pMessage, ...);
std::wstring format(const wchar_t* pMessage, ...);
} // namespace IngameIME