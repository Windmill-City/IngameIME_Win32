#include "tf/ComBSTR.hpp"

namespace IngameIME::tf
{

ComBSTR::~ComBSTR()
{
    SysFreeString(bstr);
}

[[nodiscard]] BSTR* ComBSTR::operator&()
{
    if (bstr) throw new std::runtime_error("Acquire address for non-null pointer");

    return &bstr;
}

ComBSTR::operator bool() const noexcept
{
    return bstr != nullptr;
}

} // namespace IngameIME::tf
