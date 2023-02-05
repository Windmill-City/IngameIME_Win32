#pragma once
#include "ImmInputContextImpl.hpp"

namespace IngameIME::imm
{
class CompositionImpl : public Composition
{
  protected:
    InternalRect      rect;
    InputContextImpl* inputCtx;

  public:
    CompositionImpl(InputContextImpl* inputCtx);

  public:
    /**
     * @brief Bounding box of the PreEdit text drawn by the Application, the position is in window coordinate
     *
     */
    virtual void        setPreEditRect(const PreEditRect& rect) override;
    /**
     * @brief Bounding box of the PreEdit text drawn by the Application, the position is in window coordinate
     *
     */
    virtual PreEditRect getPreEditRect() override;
    /**
     * @brief Terminate active composition
     *
     */
    virtual void terminate() noexcept override;
};
} // namespace IngameIME::imm
