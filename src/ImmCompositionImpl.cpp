#include "imm\ImmCompositionImpl.hpp"

namespace IngameIME::imm
{
CompositionImpl::CompositionImpl(InputContextImpl* inputCtx)
    : inputCtx(inputCtx)
{
}

void CompositionImpl::setPreEditRect(const PreEditRect& rect)
{
    this->rect = rect;

    inputCtx->setPreEditRect(this->rect);
}

PreEditRect CompositionImpl::getPreEditRect()
{
    return this->rect;
}

void CompositionImpl::terminate() noexcept
{
    ImmNotifyIME(inputCtx->ctx, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
}

} // namespace IngameIME::imm
