#pragma once
#include <stdexcept>

#include "FormatUtil.hpp"

namespace IngameIME::tf
{
class ComException : public std::runtime_error
{
  public:
    const HRESULT hr;

  public:
    ComException(HRESULT hr);
};
} // namespace IngameIME::tf
