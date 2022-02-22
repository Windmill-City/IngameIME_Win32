#pragma once
#include <exception>
#include <winnt.h>

namespace libtf {
    class ComException : public std::exception {
      protected:
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

      public:
        const HRESULT hr;
        ComException(HRESULT hr)
            : hr(hr), std::exception(format("ComException[0x%1!x!]: %2!s!", hr, format(hr).c_str()).c_str())
        {
        }
    };
}// namespace libtf
