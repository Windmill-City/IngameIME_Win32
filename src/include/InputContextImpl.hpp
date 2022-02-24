#pragma once
#include <map>

#include <Windows.h>
#pragma comment(lib, "imm32.lib")

#include <msctf.h>

#include "InputContext.hpp"
#include "InputProcessor.hpp"

#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "IThreadAssociate.hpp"
#include "TfFunction.hpp"

namespace IngameIME {
    struct InternalRect : public IngameIME::PreEditRect
    {
        InternalRect() = default;

        operator RECT() noexcept
        {
            RECT rect;
            rect.left   = this->left;
            rect.top    = this->top;
            rect.right  = this->right;
            rect.bottom = this->bottom;

            return rect;
        }
    };
}// namespace IngameIME

namespace libtf {
    class InputContextImpl : public IngameIME::InputContext, public IThreadAssociate {
      protected:
        class ContextOwner;

      protected:
        ComPtr<ITfThreadMgr>   threadMgr;
        TfClientId             clientId{TF_CLIENTID_NULL};
        ComPtr<ITfDocumentMgr> docMgr;
        ComPtr<ITfDocumentMgr> emptyDocMgr;
        ComPtr<ITfContext>     ctx;

        HWND                 hWnd;
        ComPtr<ContextOwner> owner;

        bool activated{false};
        bool fullscreen{false};

        friend class CompositionImpl;

      private:
        DWORD cookie{TF_INVALID_COOKIE};

      protected:
        class ContextOwner : protected ComObjectBase, public ITfContextOwner {
          protected:
            InputContextImpl* ctx;

          public:
            ContextOwner(InputContextImpl* ctx) : ctx(ctx) {}

          public:
            COM_DEF_BEGIN();
            COM_DEF_INF(ITfContextOwner);
            COM_DEF_END();

          public:
            /**
             * @brief Our implementation does not calculate a text layout
             *
             * @return HRESULT TS_E_NOLAYOUT
             */
            HRESULT STDMETHODCALLTYPE GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override
            {
                return TS_E_NOLAYOUT;
            }

            /**
             * @brief Input method call this method to get the bounding box, in screen coordinates, of preedit string,
             * and use which to position its candidate window
             */
            HRESULT STDMETHODCALLTYPE GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override
            {
                // Fetch bounding box
                IngameIME::InternalRect box;
                ctx->comp->IngameIME::PreEditRectCallbackHolder::runCallback(box);
                *prc = box;

                // Map window coordinate to screen coordinate
                MapWindowPoints(ctx->hWnd, NULL, (LPPOINT)prc, 2);

                return S_OK;
            }

            /**
             * @brief Return the bounding box, in screen coordinates, of the display surface of the text stream
             */
            HRESULT STDMETHODCALLTYPE GetScreenExt(RECT* prc) override
            {
                GetWindowRect(ctx->hWnd, prc);

                return S_OK;
            }

            /**
             * @brief Obtains the status of the context.
             */
            HRESULT STDMETHODCALLTYPE GetStatus(TF_STATUS* pdcs) override
            {
                // Set to 0 indicates the context is editable
                pdcs->dwDynamicFlags = 0;
                // Set to 0 indicates the context only support single selection
                pdcs->dwStaticFlags = 0;
                return S_OK;
            }

            /**
             * @brief Return the window handle the InputContext is associated to
             *
             * Retrive this value by calling ITfContextView::GetWnd, ITfContextView can query from ITfContext
             */
            HRESULT STDMETHODCALLTYPE GetWnd(HWND* phwnd) override
            {
                *phwnd = ctx->hWnd;

                return S_OK;
            }

            /**
             * @brief Our implementation doesn't support any attributes, just return VT_EMPTY
             */
            HRESULT STDMETHODCALLTYPE GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue) override
            {
                pvarValue->vt = VT_EMPTY;
                return S_OK;
            }
        };

      public:
        InputContextImpl(HWND hWnd);
        ~InputContextImpl()
        {
            if (cookie != TF_INVALID_COOKIE) {
                ComQIPtr<ITfSource> source(IID_ITfSource, ctx);
                source->UnadviseSink(cookie);
                cookie = TF_INVALID_COOKIE;
                owner.reset();
            }

            if (ctx) {
                setActivated(false);
                comp.reset();
                docMgr->Pop(TF_POPF_ALL);
                ctx.reset();
            }

            if (docMgr) {
                docMgr.reset();
                emptyDocMgr.reset();
            }

            if (clientId != TF_CLIENTID_NULL) {
                threadMgr->Deactivate();
                threadMgr.reset();
                clientId = TF_CLIENTID_NULL;
            }
        }

      public:
        /**
         * @brief Set if context activated
         *
         * @param activated set to true to activate input method
         * @throw UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
         */
        virtual void setActivated(const bool activated) override
        {
            COM_HR_BEGIN(S_OK);

            CHECK_HR(assertCreatorThread());

            this->activated = activated;

            ComPtr<ITfDocumentMgr> prevDocumentMgr;
            if (activated) { CHECK_HR(threadMgr->AssociateFocus(hWnd, docMgr.get(), &prevDocumentMgr)); }
            else {
                // Focus on empty context docMgr can deactivate input method
                CHECK_HR(threadMgr->AssociateFocus(hWnd, emptyDocMgr.get(), &prevDocumentMgr));
                // Terminate active composition
                comp->terminate();
            }

            COM_HR_END();
            COM_HR_THR();
        }

        /**
         * @brief Get context activate state
         *
         * @return if the context activated
         */
        virtual bool getActivated() const override
        {
            return activated;
        }

        /**
         * @brief Set InputContext full screen state
         *
         * @param fullscreen if InputContext full screen
         */
        virtual void setFullScreen(const bool fullscreen) override
        {
            this->fullscreen = fullscreen;
        }

        /**
         * @brief Get if InputContext in full screen state
         *
         * @return true full screen mode
         * @return false window mode
         */
        virtual bool getFullScreen() const override
        {
            return fullscreen;
        }
    };
}// namespace libtf

namespace libimm {
    class InputContextImpl : public IngameIME::InputContext, public libtf::IThreadAssociate {
      protected:
        static std::map<HWND, std::weak_ptr<InputContextImpl>> InputCtxMap;
        static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

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
        std::list<std::wstring> getInputModes();

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
        IngameIME::InputProcessorContext getInputProcCtx();

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