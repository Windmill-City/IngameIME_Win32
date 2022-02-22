#pragma once
#include <windef.h>
#include <wtypes.h>

#include <msctf.h>

#include "InputContext.hpp"

#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "IThreadAssociate.hpp"
#include "TfFunction.hpp"

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

        bool activated;
        bool fullscreen;

        friend class CompositionImpl;
        friend class GlobalImpl;

      private:
        DWORD cookie{TF_INVALID_COOKIE};

      protected:
        class ContextOwner : protected ComObjectBase, public ITfContextOwner {
          protected:
            struct InternalRect : public IngameIME::PreEditRect
            {
                InternalRect() = default;

                InternalRect(const RECT& rect) noexcept
                {
                    this->left   = rect.left;
                    this->top    = rect.top;
                    this->right  = rect.right;
                    this->bottom = rect.bottom;
                }

                /**
                 * @brief Assign the value of this rect to another rect
                 *
                 * @param rect the rect to assign to
                 */
                void assignTo(RECT& rect) noexcept
                {
                    rect.left   = this->left;
                    rect.top    = this->top;
                    rect.right  = this->right;
                    rect.bottom = this->bottom;
                }
            };

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
                auto box = std::make_unique<InternalRect>(*prc);
                ctx->comp->IngameIME::PreEditRectCallbackHolder::runCallback(*box);
                box->assignTo(*prc);

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
        ~InputContextImpl()
        {
            if (cookie != TF_INVALID_COOKIE) {
                ComQIPtr<ITfSource> source(IID_ITfSource, ctx);
                source->UnadviseSink(cookie);
                cookie = TF_INVALID_COOKIE;
                owner.reset();
            }

            if (ctx) {
                docMgr->Pop(TF_POPF_ALL);
                ctx.reset();
            }

            if (docMgr) {
                setActivated(false);
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