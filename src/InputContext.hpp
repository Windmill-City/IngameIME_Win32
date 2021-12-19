#pragma once
#include <msctf.h>

#include "libtfdef.h"

#include "IThreadAssociate.hpp"
#include "TfFunction.hpp"

#include "CompositionHandler.hpp"
#include "FullScreenUIElementHandler.hpp"
#include "InputProcessorHandler.hpp"
namespace libtf {
    class InputContext : public CComObjectRoot, protected ITfContextOwner, public IThreadAssociate {
        CComPtr<ITfThreadMgr>   m_ThreadMgr;
        TfClientId              m_ClientId = 0;
        CComPtr<ITfDocumentMgr> m_DocumentMgr;
        CComPtr<ITfDocumentMgr> m_EmptyContextDocumentMgr;
        CComPtr<ITfContext>     m_Context;
        DWORD                   m_ContextOwnerCookie = TF_INVALID_COOKIE;
        HWND                    m_hWnd;
        bool                    m_ContextActivated;

      public:
        const CComPtr<CFullScreenUIElementHandler> m_FullScreenUIElementHandler = new CFullScreenUIElementHandler();
        const CComPtr<CCompositionHandler>         m_CompositionHandler         = new CCompositionHandler();
        const CComPtr<CInputProcessorHandler>      m_InputProcessorHandler      = new CInputProcessorHandler();

      public:
        /**
         * @brief Create input context for specific window
         *
         * @param hWnd handle of the window to create the context
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the window
         */
        HRESULT ctor(const HWND hWnd)
        {
            BEGIN_HRESULT();

            if (!(m_hWnd = hWnd)) return E_INVALIDARG;
            if (initialCreatorThread() != GetWindowThreadProcessId(hWnd, NULL)) return UI_E_WRONG_THREAD;

            BEGIN_HRESULT_SCOPE();

            CHECK_HR(getThreadMgr(&m_ThreadMgr));

            CComQIPtr<ITfThreadMgrEx> threadMgrEx = m_ThreadMgr;
            CHECK_HR(threadMgrEx->ActivateEx(&m_ClientId, TF_TMAE_UIELEMENTENABLEDONLY));

            CHECK_HR(m_ThreadMgr->CreateDocumentMgr(&m_EmptyContextDocumentMgr));
            CHECK_HR(m_ThreadMgr->CreateDocumentMgr(&m_DocumentMgr));

            // Deactivate input contxt
            CHECK_HR(setActivated(false));

            // This EditCookie is useless
            TfEditCookie ec;
            CHECK_HR(m_DocumentMgr->CreateContext(
                m_ClientId, 0, (ITfContextOwnerCompositionSink*)m_CompositionHandler, &m_Context, &ec));
            CHECK_HR(m_DocumentMgr->Push(m_Context));

            CComQIPtr<ITfSource> source = m_Context;
            CHECK_HR(source->AdviseSink(IID_ITfContextOwner, this, &m_ContextOwnerCookie));

            CHECK_HR(m_FullScreenUIElementHandler->ctor(m_ThreadMgr));
            CHECK_HR(m_CompositionHandler->ctor(m_ClientId, m_Context));
            CHECK_HR(m_InputProcessorHandler->ctor(m_ClientId, m_ThreadMgr));

            return hr;
            END_HRESULT_SCOPE();
            dtor();
            END_HRESULT();
        }

        /**
         * @brief Cleanup input context
         *
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
         */
        HRESULT dtor()
        {
            BEGIN_HRESULT();

            if (FAILED(assertCreatorThread())) return UI_E_WRONG_THREAD;

            CHECK_HR(m_InputProcessorHandler->dtor());
            CHECK_HR(m_CompositionHandler->dtor());
            CHECK_HR(m_FullScreenUIElementHandler->dtor());

            if (m_ContextOwnerCookie != TF_INVALID_COOKIE) {
                CComQIPtr<ITfSource> source = m_Context;
                CHECK_HR(source->UnadviseSink(m_ContextOwnerCookie));
                m_ContextOwnerCookie = TF_INVALID_COOKIE;
            }

            if (m_Context) {
                CHECK_HR(m_DocumentMgr->Pop(TF_POPF_ALL));
                m_Context.Release();
            }

            if (m_DocumentMgr) {
                CHECK_HR(setActivated(false));
                m_DocumentMgr.Release();
                m_EmptyContextDocumentMgr.Release();
            }

            if (m_ClientId) {
                CHECK_HR(m_ThreadMgr->Deactivate());
                m_ThreadMgr.Release();
                m_ClientId = 0;
            }

            END_HRESULT();
        }

      public:
        BEGIN_COM_MAP(InputContext)
        COM_INTERFACE_ENTRY(ITfContextOwner)
        END_COM_MAP()

        /**
         * @brief Set if context activated
         *
         * @param activated set to true to activate input method
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
         */
        HRESULT setActivated(const bool activated)
        {
            BEGIN_HRESULT();

            CHECK_HR(assertCreatorThread());

            m_ContextActivated = activated;

            CComPtr<ITfDocumentMgr> prevDocumentMgr;
            if (m_ContextActivated) { CHECK_HR(m_ThreadMgr->AssociateFocus(m_hWnd, m_DocumentMgr, &prevDocumentMgr)); }
            else {
                // Focus on empty context documentMgr can deactivate input method
                CHECK_HR(m_ThreadMgr->AssociateFocus(m_hWnd, m_EmptyContextDocumentMgr, &prevDocumentMgr));
            }

            END_HRESULT();
        }

        /**
         * @brief Get context activate state
         *
         * @param activated receive context activate state
         * @return E_INVALIDARG if activated is NULL
         */
        HRESULT getActivated(bool* activated) const
        {
            if (!activated) return E_INVALIDARG;
            *activated = m_ContextActivated;
            return S_OK;
        }

#pragma region ITfContextOwner
        /**
         * @brief We will not use this method
         *
         * @return HRESULT TS_E_NOLAYOUT - The Application not calculate a text layout
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
            m_CompositionHandler->m_PreEditHandler->PreEditRectCallback::runCallback(std::forward<RECT*>(prc));
            // Map window coordinate to screen coordinate
            MapWindowPoints(m_hWnd, NULL, (LPPOINT)prc, 2);
            return S_OK;
        }

        /**
         * @brief Return the bounding box, in screen coordinates, of the display surface of the text stream
         */
        HRESULT STDMETHODCALLTYPE GetScreenExt(RECT* prc) override
        {
            GetWindowRect(m_hWnd, prc);
            return S_OK;
        }

        /**
         * @brief Obtains the status of a document.
         */
        HRESULT STDMETHODCALLTYPE GetStatus(TF_STATUS* pdcs) override
        {
            // Current context is editable
            pdcs->dwDynamicFlags = 0;
            // Current context only support single selection
            pdcs->dwStaticFlags = 0;
            return S_OK;
        }

        /**
         * @brief Return current focused window,
         * can be retrieve by ITfContextView, which can query from ITfContext
         */
        HRESULT STDMETHODCALLTYPE GetWnd(HWND* phwnd) override
        {
            phwnd = &m_hWnd;
            return S_OK;
        }

        /**
         * @brief Doesn't support any attributes, just return VT_EMPTY
         */
        HRESULT STDMETHODCALLTYPE GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue) override
        {
            pvarValue->vt = VT_EMPTY;
            return S_OK;
        }
#pragma endregion
    };

    typedef CComObjectNoLock<InputContext> CInputContext;
}// namespace libtf