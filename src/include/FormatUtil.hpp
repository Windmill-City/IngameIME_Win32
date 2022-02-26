#pragma once
#include <string>

#include <windows.h>

std::string format(const HRESULT hr)
{
    LPSTR pBufHr = NULL;

    FormatMessageA(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                   NULL,
                   hr,
                   MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                   (LPSTR)&pBufHr,
                   0,
                   NULL);

    auto result = std::string(pBufHr);
    LocalFree(pBufHr);

    return result;
}

std::string format(const char* pMessage, ...)
{
    va_list args = NULL;
    va_start(args, pMessage);

    LPSTR pBufMsg = NULL;

    FormatMessageA(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER, pMessage, 0, 0, (LPSTR)&pBufMsg, 0, &args);

    va_end(args);

    auto result = std::string(pBufMsg);
    LocalFree(pBufMsg);

    return result;
}

std::wstring format(const wchar_t* pMessage, ...)
{
    va_list args = NULL;
    va_start(args, pMessage);

    LPWSTR pBufMsg = NULL;

    FormatMessageW(
        FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER, pMessage, 0, 0, (LPWSTR)&pBufMsg, 0, &args);

    va_end(args);

    auto result = std::wstring(pBufMsg);
    LocalFree(pBufMsg);

    return result;
}