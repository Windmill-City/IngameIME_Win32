#pragma once
#include <string>

#include <windows.h>

std::wstring GetMessageText(UINT msg) noexcept;

void ShowUsedMsg() noexcept;

void ShowMessageText(UINT msg) noexcept;