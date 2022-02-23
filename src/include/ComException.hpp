#pragma once
#include <exception>

#include "FormatUtil.hpp"

namespace libtf {

    class ComException : public std::exception {
      public:
        const HRESULT hr;
        ComException(HRESULT hr)
            : hr(hr), std::exception(format("ComException[0x%1!08x!]: %2!s!", hr, format(hr).c_str()).c_str())
        {
        }
    };
}// namespace libtf
