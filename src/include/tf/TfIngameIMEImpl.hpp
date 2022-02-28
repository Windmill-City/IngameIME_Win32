#pragma once
#include "IngameIME.hpp"

#include "TfInputContextImpl.hpp"
#include "TfInputProcessorImpl.hpp"

namespace libtf {
    class GlobalImpl : public IngameIME::Global {
      private:
        DWORD cookieComp{TF_INVALID_COOKIE};
        DWORD cookieProc{TF_INVALID_COOKIE};

      protected:
        ComPtr<InputProcessorHandler> handler;
        friend class IngameIME::Global;

      protected:
        GlobalImpl()
        {
            COM_HR_BEGIN(S_OK);

            handler = new InputProcessorHandler();

            ComQIPtr<ITfSource> source(IID_ITfSource, handler->mode);
            CHECK_HR(source->AdviseSink(
                IID_ITfCompartmentEventSink, static_cast<ITfCompartmentEventSink*>(handler.get()), &cookieComp));

            source = handler->compMgr;
            CHECK_HR(source->AdviseSink(IID_ITfInputProcessorProfileActivationSink,
                                        static_cast<ITfInputProcessorProfileActivationSink*>(handler.get()),
                                        &cookieProc));

            COM_HR_END();
            COM_HR_THR();
        }

        ~GlobalImpl()
        {
            if (cookieProc != TF_INVALID_COOKIE) {
                ComQIPtr<ITfSource> source(IID_ITfSource, handler);
                source->UnadviseSink(cookieProc);
                cookieProc = TF_INVALID_COOKIE;
            }

            if (cookieComp != TF_INVALID_COOKIE) {
                ComQIPtr<ITfSource> source(IID_ITfSource, handler->mode);
                source->UnadviseSink(cookieComp);
                cookieComp = TF_INVALID_COOKIE;
            }
        }

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
         * @param hWnd the window to create InputContext
         * @return std::shared_ptr<InputContext>
         */
        virtual std::shared_ptr<IngameIME::InputContext> getInputContext(void* hWnd, ...) override
        {
            return std::make_shared<InputContextImpl>(reinterpret_cast<HWND>(hWnd));
        }
    };
}// namespace libtf