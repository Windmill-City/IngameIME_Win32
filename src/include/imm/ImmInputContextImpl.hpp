#pragma once
#include <map>

#include <windows.h>
#pragma comment(lib, "imm32.lib")

#include "IngameIME.hpp"
#include "InputContextImpl.hpp"
#include "InputProcessorImpl.hpp"

namespace libimm {
    class InputContextImpl : public IngameIME::InputContext {
      protected:
        static std::map<HWND, std::weak_ptr<InputContextImpl>> InputCtxMap;
        static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
        {
            auto iter = InputCtxMap.find(hWnd);

            std::shared_ptr<InputContextImpl> inputCtx;
            if (iter != InputCtxMap.end() && (inputCtx = (*iter).second.lock())) {
                switch (msg) {
                    case WM_INPUTLANGCHANGE:
                        IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::FullUpdate,
                                                                     inputCtx->getInputProcCtx());
                        break;
                    case WM_IME_SETCONTEXT:
                        // We should always hide Composition Window to make the PreEditCallback for work
                        lparam &= ~ISC_SHOWUICOMPOSITIONWINDOW;

                        if (inputCtx->fullscreen) {
                            // Hide Candidate Window
                            lparam &= ~ISC_SHOWUICANDIDATEWINDOW;
                        }
                        break;
                    case WM_IME_STARTCOMPOSITION:
                        inputCtx->comp->IngameIME::PreEditCallbackHolder::runCallback(
                            IngameIME::CompositionState::Begin, nullptr);
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
                        inputCtx->comp->IngameIME::CandidateListCallbackHolder::runCallback(
                            IngameIME::CandidateListState::End, nullptr);
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
                            IngameIME::Global::getInstance().runCallback(
                                IngameIME::InputProcessorState::InputModeUpdate, inputCtx->getInputProcCtx());
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

      protected:
        HWND    hWnd;
        WNDPROC prevProc;

        HIMC ctx;

        bool activated{false};
        bool fullscreen{false};

        friend class CompositionImpl;
        friend class GlobalImpl;

      public:
        InputContextImpl(HWND hWnd);
        ~InputContextImpl()
        {
            InputCtxMap.erase(hWnd);
            comp->terminate();
            ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
            SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)prevProc);
        }

      protected:
        /**
         * @brief Retrive PreEdit info for current Composition
         *
         */
        void procPreEdit()
        {
            // PreEdit Text
            auto size = ImmGetCompositionStringW(ctx, GCS_COMPSTR, NULL, 0);
            // Error occurs
            if (size <= 0) return;

            auto buf = std::make_unique<WCHAR[]>(size / sizeof(WCHAR));
            ImmGetCompositionStringW(ctx, GCS_COMPSTR, buf.get(), size);

            // Selection
            int sel = ImmGetCompositionStringW(ctx, GCS_CURSORPOS, NULL, 0);

            IngameIME::PreEditContext ctx;
            ctx.content  = std::wstring(buf.get(), size / sizeof(WCHAR));
            ctx.selStart = ctx.selEnd = sel;

            comp->IngameIME::PreEditCallbackHolder::runCallback(IngameIME::CompositionState::Update, &ctx);
        }

        /**
         * @brief Retrive Commit text for current Composition
         *
         */
        void procCommit()
        {
            auto size = ImmGetCompositionStringW(ctx, GCS_RESULTSTR, NULL, 0);
            // Error occurs
            if (size <= 0) return;

            auto buf = std::make_unique<WCHAR[]>(size / sizeof(WCHAR));
            ImmGetCompositionStringW(ctx, GCS_RESULTSTR, buf.get(), size);

            comp->IngameIME::CommitCallbackHolder::runCallback(std::wstring(buf.get(), size / sizeof(WCHAR)));
        }

        /**
         * @brief Set CandidateList window's position for current Composition
         *
         */
        void procPreEditRect()
        {
            IngameIME::InternalRect rect;
            comp->IngameIME::PreEditRectCallbackHolder::runCallback(rect);

            CANDIDATEFORM cand;
            cand.dwIndex        = 0;
            cand.dwStyle        = CFS_EXCLUDE;
            cand.ptCurrentPos.x = rect.left;
            cand.ptCurrentPos.y = rect.top;
            cand.rcArea         = rect;
            ImmSetCandidateWindow(ctx, &cand);

            COMPOSITIONFORM comp;
            comp.dwStyle        = CFS_RECT;
            comp.ptCurrentPos.x = rect.left;
            comp.ptCurrentPos.y = rect.top;
            comp.rcArea         = rect;
            ImmSetCompositionWindow(ctx, &comp);
        }

        void procCand()
        {
            IngameIME::CandidateListContext candCtx;

            auto size = ImmGetCandidateListW(ctx, 0, NULL, 0);
            // Error occurs
            if (size == 0) return;

            auto buf  = std::make_unique<byte[]>(size);
            auto cand = (LPCANDIDATELIST)buf.get();

            ImmGetCandidateListW(ctx, 0, cand, size);

            auto pageSize  = cand->dwPageSize;
            auto candCount = cand->dwCount;

            auto pageStart    = cand->dwPageStart;
            auto pageEnd      = pageStart + pageSize;
            candCtx.selection = cand->dwSelection;
            // Absolute index to relative index
            candCtx.selection -= pageStart;

            for (size_t i = 0; i < pageSize; i++) {
                auto strStart = buf.get() + cand->dwOffset[i + pageStart];
                auto strEnd =
                    buf.get() + (((i + pageStart + 1) < candCount) ? cand->dwOffset[i + pageStart + 1] : size);
                auto len = (strEnd - strStart) / sizeof(WCHAR);

                candCtx.candidates.push_back(std::wstring((wchar_t*)strStart, len));
            }

            comp->IngameIME::CandidateListCallbackHolder::runCallback(IngameIME::CandidateListState::Update, &candCtx);
        }

      public:
        IngameIME::InputProcessorContext getInputProcCtx()
        {
            IngameIME::InputProcessorContext result;

            DWORD mode;
            ImmGetConversionStatus(ctx, &mode, NULL);

            auto activeProc = IngameIME::InputProcessorImpl::getActiveInputProcessor();

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

            result.proc  = activeProc;
            result.modes = modes;

            return result;
        }

      public:
        /**
         * @brief Set InputContext activate state
         *
         * @param activated if InputContext activated
         */
        virtual void setActivated(const bool activated) noexcept override
        {
            this->activated = activated;

            if (activated)
                ImmAssociateContext(hWnd, ctx);
            else
                ImmAssociateContext(hWnd, NULL);
        }
        /**
         * @brief Get if InputContext activated
         *
         * @return true activated
         * @return false not activated
         */
        virtual bool getActivated() const noexcept override
        {
            return activated;
        }
        /**
         * @brief Set InputContext full screen state
         *
         * @param fullscreen if InputContext full screen
         */
        virtual void setFullScreen(const bool fullscreen) noexcept override
        {
            this->fullscreen = fullscreen;

            if (activated) {
                comp->terminate();
                // Refresh InputContext
                ImmAssociateContext(hWnd, NULL);
                ImmAssociateContext(hWnd, ctx);
            }
        }
        /**
         * @brief Get if InputContext in full screen state
         *
         * @return true full screen mode
         * @return false window mode
         */
        virtual bool getFullScreen() const noexcept override
        {
            return fullscreen;
        }
    };
    std::map<HWND, std::weak_ptr<InputContextImpl>> InputContextImpl::InputCtxMap = {};
}// namespace libimm