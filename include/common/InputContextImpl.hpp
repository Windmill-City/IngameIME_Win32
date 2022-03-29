#pragma once
#include <windows.h>

#include "InputContext.hpp"

namespace IngameIME {
    struct InternalRect : public PreEditRect
    {
        InternalRect() = default;

        operator RECT() noexcept
        {
            RECT rect;
            rect.left   = this->left;
            rect.top    = this->top;
            rect.right  = this->right;
            rect.bottom = this->bottom;

            return rect;
        }
    };
}// namespace IngameIME