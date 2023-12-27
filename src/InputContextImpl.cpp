#include "common\InputContextImpl.hpp"

namespace IngameIME
{
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
