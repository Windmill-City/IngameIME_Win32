#pragma once
#include "ImmInputContextImpl.hpp"

namespace IngameIME::imm
{
class CompositionImpl : public Composition
{
  protected:
    InputContextImpl* inputCtx;

  public:
    CompositionImpl(InputContextImpl* inputCtx);

  public:
    /**
     * @brief Terminate active composition
     *
     */
    virtual void terminate() noexcept override;
};
} // namespace IngameIME::imm
