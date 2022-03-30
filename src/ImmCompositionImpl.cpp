#include "imm/ImmCompositionImpl.hpp"

namespace IngameIME::imm
{
CompositionImpl::CompositionImpl(InputContextImpl* inputCtx)
    : inputCtx(inputCtx)
{
}

void CompositionImpl::terminate() noexcept
{
    ImmNotifyIME(inputCtx->ctx, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
}

} // namespace IngameIME::imm
