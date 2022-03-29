#pragma once
#include "ImmInputContextImpl.hpp"

namespace IngameIME::imm {
    class CompositionImpl : public Composition {
      protected:
        InputContextImpl* inputCtx;

      public:
        CompositionImpl(InputContextImpl* inputCtx) : inputCtx(inputCtx) {}

      public:
        /**
         * @brief Terminate active composition
         *
         */
        virtual void terminate() noexcept override
        {
            ImmNotifyIME(inputCtx->ctx, NI_COMPOSITIONSTR, CPS_COMPLETE, 0);
        }
    };
}// namespace IngameIME::imm