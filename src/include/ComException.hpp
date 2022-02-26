#pragma once
#include <exception>

#include "FormatUtil.hpp"

namespace libtf {

    class ComException : public std::runtime_error {
      public:
        const HRESULT hr;
        ComException(HRESULT hr)
            : hr(hr), std::runtime_error(format("ComException[0x%1!08x!]: %2!s!", hr, format(hr).c_str()).c_str())
        {
        }
    };
}// namespace libtf
