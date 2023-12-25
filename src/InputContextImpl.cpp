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

std::string ToUTF8(std::wstring utf16)
{
    if (utf16.empty())
    {
        return std::string();
    }
    int         size_needed = WideCharToMultiByte(CP_UTF8, 0, &utf16[0], (int)utf16.size(), NULL, 0, NULL, NULL);
    std::string strTo(size_needed, 0);
    WideCharToMultiByte(CP_UTF8, 0, &utf16[0], (int)utf16.size(), &strTo[0], size_needed, NULL, NULL);
    return strTo;
}
} // namespace IngameIME
