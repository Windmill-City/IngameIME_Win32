#pragma once
#include "IngameIME.hpp"
#include "InputProcessorImpl.hpp"

#include "ImmInputContextImpl.hpp"

namespace libimm {
    class GlobalImpl : public IngameIME::Global {
      public:
        /**
         * @brief Get Active InputProcessor
         *
         * @return std::shared_ptr<InputProcessor>
         */
        virtual std::shared_ptr<const IngameIME::InputProcessor> getActiveInputProcessor() const override
        {
            return IngameIME::InputProcessorImpl::getActiveInputProcessor();
        }

        /**
         * @brief Get system availiable InputProcessor
         *
         * @return std::list<std::shared_ptr<InputProcessor>>
         */
        virtual std::list<std::shared_ptr<const IngameIME::InputProcessor>> getInputProcessors() const override
        {
            return IngameIME::InputProcessorImpl::getInputProcessors();
        }

        /**
         * @brief Get the InputContext object
         *
         * @return std::shared_ptr<InputContext>
         */
        virtual std::shared_ptr<IngameIME::InputContext> getInputContext(void* hWnd, ...) override
        {
            std::shared_ptr<InputContextImpl> ctx;

            auto iter = InputContextImpl::InputCtxMap.find(reinterpret_cast<HWND>(hWnd));
            if (iter == InputContextImpl::InputCtxMap.end() || !(ctx = (*iter).second.lock())) {
                ctx = std::make_shared<InputContextImpl>(reinterpret_cast<HWND>(hWnd));

                InputContextImpl::InputCtxMap[reinterpret_cast<HWND>(hWnd)] = ctx;
            }
            return ctx;
        }
    };
}// namespace libimm