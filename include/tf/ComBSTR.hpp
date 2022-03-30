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
     * @brief Acquire address of bstr
     *
     * this usually use for acquiring BSTR string, so the bstr must be null,
     * if you do want to acquire the address of the bstr, use &bstr instead
     *
     * @return address of the bstr
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