#include "tf\ComException.hpp"

namespace IngameIME::tf
{
static std::string GetHResultString(HRESULT hr)
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

static std::string FormatString(const char* pMessage, ...)
{
    va_list args = NULL;
    va_start(args, pMessage);

    LPSTR pBufMsg = NULL;

    FormatMessageA(FORMAT_MESSAGE_FROM_STRING | FORMAT_MESSAGE_ALLOCATE_BUFFER,
                   pMessage,
                   0,
                   MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US),
                   (LPSTR)&pBufMsg,
                   0,
                   &args);

    va_end(args);

    auto result = std::string(pBufMsg);
    LocalFree(pBufMsg);

    return result;
}

ComException::ComException(HRESULT hr)
    : hr(hr)
    , std::runtime_error(FormatString("ComException[0x%1!08x!]: %2!s!", hr, GetHResultString(hr).c_str()).c_str())
{
}
} // namespace IngameIME::tf
