#pragma once
#include <windows.h>

#include "InputContext.hpp"

namespace IngameIME
{
struct InternalRect : public PreEditRect
{
    InternalRect() = default;

    /**
     * @brief Implicit convert to RECT
     *
     * @return RECT
     */
    operator RECT() noexcept;
};
} // namespace IngameIME