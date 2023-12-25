#pragma once
#include <stdexcept>

#include <windows.h>

namespace IngameIME::tf
{
class ComBSTR
{
  public:
    BSTR bstr{nullptr};

  public:
    constexpr ComBSTR() = default;

    constexpr ComBSTR(std::nullptr_t) noexcept
    {
    }

    ~ComBSTR();

    /**
     * @brief this method only for receiving a new BSTR string, 'bstr' is expected to be null
     */
    [[nodiscard]] BSTR* operator&();
    /**
     * @brief Check if the BSTR is nullptr
     *
     * @return true
     * @return false
     */
    explicit            operator bool() const noexcept;
};
} // namespace IngameIME::tf