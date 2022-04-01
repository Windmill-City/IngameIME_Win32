#include "tf\ComException.hpp"

namespace IngameIME::tf
{
ComException::ComException(HRESULT hr)
    : hr(hr)
    , std::runtime_error(format("ComException[0x%1!08x!]: %2!s!", hr, format(hr).c_str()).c_str())
{
}
} // namespace IngameIME::tf
