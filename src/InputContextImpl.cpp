#include "common\InputContextImpl.hpp"

namespace IngameIME
{
InternalRect::operator RECT() noexcept
{
    RECT rect;
    rect.left   = this->left;
    rect.top    = this->top;
    rect.right  = this->right;
    rect.bottom = this->bottom;

    return rect;
}
} // namespace IngameIME
