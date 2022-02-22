#pragma once
#include <windows.h>

#include <exception>
#include <oleauto.h>

namespace libtf {
    class ComBSTR {
      public:
        BSTR bstr{nullptr};

      public:
        constexpr ComBSTR() = default;
        constexpr ComBSTR(std::nullptr_t) noexcept {}

        ~ComBSTR()
        {
            SysFreeString(bstr);
        }

        /**
         * @brief Acquire address of bstr
         *
         * this usually use for acquiring BSTR string, so the bstr must be null,
         * if you do want to acquire the address of the bstr, use &bstr instead
         *
         * @return address of the bstr
         */
        _NODISCARD BSTR* operator&()
        {
            if (bstr) throw new std::exception("Acquire address for non-null pointer");
            return &bstr;
        }

        explicit operator bool() const noexcept
        {
            return bstr != nullptr;
        }
    };
}// namespace libtf