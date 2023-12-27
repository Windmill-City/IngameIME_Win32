#pragma once
#include <windows.h>

#include "IngameIME.hpp"

namespace IngameIME
{
std::string ToUTF8(std::wstring utf16);
} // namespace IngameIME