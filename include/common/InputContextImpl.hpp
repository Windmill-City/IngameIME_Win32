#pragma once
#include <windows.h>

#include "IngameIME.hpp"

namespace IngameIME
{
struct InternalRect : public PreEditRect
{
    InternalRect() = default;
    InternalRect(const PreEditRect& rect);

    /**
     * @brief Implicit convert to RECT
     *
     * @return RECT
     */
    operator RECT() noexcept;
};

std::string ToUTF8(std::wstring utf16);
} // namespace IngameIME