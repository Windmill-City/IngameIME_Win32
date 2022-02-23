#include <stdarg.h>

#include <Windows.h>

#include <WinUser.h>

#include "CompositionImpl.hpp"
#include "IngameIME.hpp"
#include "InputProcessorImpl.hpp"

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
            return std::make_shared<InputContextImpl>(reinterpret_cast<HWND>(hWnd));
        }
    };
}// namespace libtf

libtf::InputContextImpl::InputContextImpl(HWND hWnd)
{
    COM_HR_BEGIN(S_OK);

    if (!(this->hWnd = reinterpret_cast<HWND>(hWnd))) THR_HR(E_INVALIDARG);
    if (initialCreatorThread() != GetWindowThreadProcessId(hWnd, NULL)) THR_HR(UI_E_WRONG_THREAD);

    CHECK_HR(getThreadMgr(&threadMgr));

    ComQIPtr<ITfThreadMgrEx> threadMgrEx(IID_ITfThreadMgrEx, threadMgr);
    CHECK_HR(threadMgrEx->ActivateEx(&clientId, TF_TMAE_UIELEMENTENABLEDONLY));

    CHECK_HR(threadMgr->CreateDocumentMgr(&emptyDocMgr));
    CHECK_HR(threadMgr->CreateDocumentMgr(&docMgr));

    comp = std::make_shared<CompositionImpl>(this);
    CHECK_HR(docMgr->Push(ctx.get()));

    // Deactivate input contxt
    setActivated(false);

    ComQIPtr<ITfSource> source(IID_ITfSource, ctx);
    owner = new InputContextImpl::ContextOwner(this);
    CHECK_HR(source->AdviseSink(IID_ITfContextOwner, owner.get(), &cookie));

    COM_HR_END();
    COM_HR_THR();
}

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
            return InputProcessorImpl::getActiveInputProcessor();
        }

        /**
         * @brief Get system availiable InputProcessor
         *
         * @return std::list<std::shared_ptr<InputProcessor>>
         */
        virtual std::list<std::shared_ptr<const IngameIME::InputProcessor>> getInputProcessors() const override
        {
            return InputProcessorImpl::getInputProcessors();
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

libimm::InputContextImpl::InputContextImpl(HWND hWnd)
{
    comp     = std::make_shared<CompositionImpl>(this);
    ctx      = ImmAssociateContext(hWnd, NULL);
    prevProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)InputContextImpl::WndProc);
}

std::list<std::wstring> libimm::InputContextImpl::getInputModes()
{
    DWORD mode;
    ImmGetConversionStatus(ctx, &mode, NULL);

    auto activeProc = InputProcessorImpl::getActiveInputProcessor();

    std::list<std::wstring> modes;
    if (activeProc->type == IngameIME::InputProcessorType::KeyboardLayout)
        modes.push_back(L"AlphaNumeric");
    else {
        if (mode & IME_CMODE_NATIVE) {
            modes.push_back(L"Native");

            if (activeProc->isJap)
                if (mode & IME_CMODE_KATAKANA)
                    modes.push_back(L"Katakana");
                else
                    modes.push_back(L"Hiragana");
        }
        else
            modes.push_back(L"AlphaNumeric");

        if (mode & IME_CMODE_FULLSHAPE)
            modes.push_back(L"FullShape");
        else
            modes.push_back(L"HalfShape");
    }

    return modes;
}

IngameIME::InputProcessorContext libimm::InputContextImpl::getInputProcCtx()
{
    IngameIME::InputProcessorContext result;

    result.proc  = InputProcessorImpl::getActiveInputProcessor();
    result.modes = getInputModes();

    return result;
}

LRESULT libimm::InputContextImpl::WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    auto iter = InputCtxMap.find(hWnd);

    std::shared_ptr<InputContextImpl> inputCtx;
    if (iter != InputCtxMap.end() && (inputCtx = (*iter).second.lock())) {
        switch (msg) {
            case WM_GETDLGCODE:
                // Allow InputMethod get keys
                return DLGC_WANTALLKEYS;
            case WM_INPUTLANGCHANGE:
                IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::FullUpdate,
                                                             inputCtx->getInputProcCtx());
                break;
            case WM_IME_SETCONTEXT:
                // wparam indicates window active state, when it is true, we should associate the ctx
                // if the inputCtx is activated
                if (wparam && inputCtx->activated)
                    ImmAssociateContext(hWnd, inputCtx->ctx);
                else
                    ImmAssociateContext(hWnd, NULL);

                // We should always hide Composition Window to make the PreEditCallback for work
                lparam &= ~ISC_SHOWUICOMPOSITIONWINDOW;

                if (inputCtx->fullscreen) {
                    // Hide Candidate Window
                    lparam &= ~ISC_SHOWUICANDIDATEWINDOW;
                }
                break;
            case WM_IME_STARTCOMPOSITION:
                inputCtx->comp->IngameIME::PreEditCallbackHolder::runCallback(IngameIME::CompositionState::Begin,
                                                                              nullptr);
                return true;
            case WM_IME_COMPOSITION:
                if (lparam & (GCS_COMPSTR | GCS_CURSORPOS)) inputCtx->procPreEdit();
                if (lparam & GCS_RESULTSTR) inputCtx->procCommit();

                if (!inputCtx->fullscreen) inputCtx->procPreEditRect();

                // when lparam == 0 that means current Composition has been canceled
                if (lparam) return true;
            case WM_IME_ENDCOMPOSITION:
                inputCtx->comp->IngameIME::PreEditCallbackHolder::runCallback(IngameIME::CompositionState::End,
                                                                              nullptr);
                inputCtx->comp->IngameIME::CandidateListCallbackHolder::runCallback(IngameIME::CandidateListState::End,
                                                                                    nullptr);
                return true;
            case WM_IME_NOTIFY:
                if (inputCtx->fullscreen) switch (wparam) {
                        case IMN_OPENCANDIDATE:
                            inputCtx->comp->IngameIME::CandidateListCallbackHolder::runCallback(
                                IngameIME::CandidateListState::Begin, nullptr);
                            return true;
                        case IMN_CHANGECANDIDATE: inputCtx->procCand(); return true;
                        case IMN_CLOSECANDIDATE:
                            inputCtx->comp->IngameIME::CandidateListCallbackHolder::runCallback(
                                IngameIME::CandidateListState::End, nullptr);
                            return true;
                        default: break;
                    }
                if (wparam == IMN_SETCONVERSIONMODE) {
                    IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::InputModeUpdate,
                                                                 inputCtx->getInputProcCtx());
                }
                break;
            case WM_IME_CHAR:
                // Commit text already handled at WM_IME_COMPOSITION
                return true;
            default: return CallWindowProcW(inputCtx->prevProc, hWnd, msg, wparam, lparam);
        }
    }
    return DefWindowProcW(hWnd, msg, wparam, lparam);
}

#include <VersionHelpers.h>
IngameIME::Global& IngameIME::Global::getInstance()
{
    thread_local IngameIME::Global& Instance = IsWindows8OrGreater() ? (IngameIME::Global&)*new libtf::GlobalImpl() :
                                                                       (IngameIME::Global&)*new libimm::GlobalImpl();
    return Instance;
}