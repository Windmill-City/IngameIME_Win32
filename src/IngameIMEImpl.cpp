#include "CompositionImpl.hpp"
#include "IngameIME.hpp"
#include "InputProcessorImpl.hpp"

#include <stdarg.h>

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
            return handler->activeProc;
        }

        /**
         * @brief Get system availiable InputProcessor
         *
         * @return std::list<std::shared_ptr<InputProcessor>>
         */
        virtual std::list<std::shared_ptr<const IngameIME::InputProcessor>> getInputProcessors() const override
        {
            std::list<std::shared_ptr<const IngameIME::InputProcessor>> result;

            COM_HR_BEGIN(S_OK);

            if (!IsGUIThread(false)) break;

            ComPtr<ITfInputProcessorProfiles> profiles;
            CHECK_HR(createInputProcessorProfiles(&profiles));
            ComQIPtr<ITfInputProcessorProfileMgr> procMgr(IID_ITfInputProcessorProfileMgr, profiles);

            ComPtr<IEnumTfInputProcessorProfiles> enumProfiles;
            // Pass 0 to langid to enum all profiles
            CHECK_HR(procMgr->EnumProfiles(0, &enumProfiles));

            TF_INPUTPROCESSORPROFILE profile[1];
            while (true) {
                ULONG fetch;
                CHECK_HR(enumProfiles->Next(1, profile, &fetch));

                // No more
                if (fetch == 0) break;

                // InputProcessor not enabled can't be activated
                if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED)) continue;

                result.push_back(InputProcessorImpl::getInputProcessor(profile[0]));
            }
            COM_HR_END();

            return result;
        }

        /**
         * @brief Get the InputContext object
         *
         * @param hWnd the window to create InputContext
         * @return std::shared_ptr<InputContext>
         */
        virtual std::shared_ptr<IngameIME::InputContext> getInputContext(void* hWnd, ...) override
        {
            auto inputCtx = std::make_shared<InputContextImpl>();

            COM_HR_BEGIN(S_OK);

            if (!(inputCtx->hWnd = reinterpret_cast<HWND>(hWnd))) THR_HR(E_INVALIDARG);
            if (inputCtx->initialCreatorThread() != GetWindowThreadProcessId(inputCtx->hWnd, NULL))
                THR_HR(UI_E_WRONG_THREAD);

            CHECK_HR(getThreadMgr(&inputCtx->threadMgr));

            ComQIPtr<ITfThreadMgrEx> threadMgrEx(IID_ITfThreadMgrEx, inputCtx->threadMgr);
            CHECK_HR(threadMgrEx->ActivateEx(&inputCtx->clientId, TF_TMAE_UIELEMENTENABLEDONLY));

            CHECK_HR(inputCtx->threadMgr->CreateDocumentMgr(&inputCtx->emptyDocMgr));
            CHECK_HR(inputCtx->threadMgr->CreateDocumentMgr(&inputCtx->docMgr));

            inputCtx->comp = std::make_shared<CompositionImpl>(inputCtx.get());
            CHECK_HR(inputCtx->docMgr->Push(inputCtx->ctx.get()));

            // Deactivate input contxt
            inputCtx->setActivated(false);

            ComQIPtr<ITfSource> source(IID_ITfSource, inputCtx->ctx);
            inputCtx->owner = new InputContextImpl::ContextOwner(inputCtx.get());
            CHECK_HR(source->AdviseSink(IID_ITfContextOwner, inputCtx->owner.get(), &inputCtx->cookie));

            COM_HR_END();
            COM_HR_THR();

            return inputCtx;
        }
    };
}// namespace libtf

IngameIME::Global& IngameIME::Global::getInstance()
{
    thread_local IngameIME::Global& Instance = *new libtf::GlobalImpl();
    return Instance;
}