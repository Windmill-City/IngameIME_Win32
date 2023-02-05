#include "common\InputContextImpl.hpp"

namespace IngameIME
{
InternalRect::operator RECT() noexcept
{
    RECT rect;
    rect.left   = this->x;
    rect.top    = this->y;
    rect.right  = this->x + this->width;
    rect.bottom = this->y + this->height;

    return rect;
}

InternalRect::InternalRect(const PreEditRect& rect)
{
    this->x      = rect.x;
    this->y      = rect.y;
    this->width  = rect.width;
    this->height = rect.height;
}
} // namespace IngameIME
