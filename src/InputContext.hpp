#pragma once
#include "TfFunction.hpp"
#include "libtfdef.h"

#include "CandidateListHandler.hpp"
#include "CompositionHandler.hpp"
#include "ConversionModeHandler.hpp"
#include "FullScreenUIElementHandler.hpp"
#include "InputProcessorHandler.hpp"
#include "SentenceModeHandler.hpp"

#include <msctf.h>

namespace libtf {
    class InputContext : public CComObjectRoot, protected ITfContextOwner {
      protected:
        CComPtr<ITfThreadMgr>   m_threadMgr;
        TfClientId              m_clientId;
        CComPtr<ITfDocumentMgr> m_documentMgr;
        CComPtr<ITfContext>     m_context;
        DWORD                   m_contextOwnerCookie;
        HWND                    m_hWnd = NULL;
        bool                    m_imState;

      public:
        CComPtr<CCandidateListHandler>       m_candHandler        = new CCandidateListHandler();
        CComPtr<CFullScreenUIElementHandler> m_fullScHandler      = new CFullScreenUIElementHandler();
        CComPtr<CCompositionHandler>         m_compositionHandler = new CCompositionHandler();
        CComPtr<CConversionModeHandler>      m_conversionHander   = new CConversionModeHandler();
        CComPtr<CSentenceModeHandler>        m_sentenceHander     = new CSentenceModeHandler();
        CComPtr<CInputProcessorHandler>      m_inputProcessor     = new CInputProcessorHandler();

        BEGIN_COM_MAP(InputContext)
        COM_INTERFACE_ENTRY(ITfContextOwner)
        END_COM_MAP()

        /**
         * @brief Initialize InputContext on the calling thread
         */
        HRESULT initialize()
        {
            CHECK_HR(createThreadMgr(&m_threadMgr));

            CComQIPtr<ITfThreadMgrEx> threadMgrEx = m_threadMgr;
            CHECK_HR(threadMgrEx->ActivateEx(&m_clientId, TF_TMAE_UIELEMENTENABLEDONLY));

            CHECK_HR(setIMState(false));

            CHECK_HR(m_threadMgr->CreateDocumentMgr(&m_documentMgr));

            TfEditCookie ec;
            CHECK_HR(m_documentMgr->CreateContext(
                m_clientId, 0, (ITfContextOwnerCompositionSink*)m_compositionHandler, &m_context, &ec));
            CHECK_HR(m_documentMgr->Push(m_context));

            HRESULT                      hr;
            CComQIPtr<ITfUIElementMgr>   uiElementMgr   = m_threadMgr;
            CComQIPtr<ITfCompartmentMgr> compartmentMgr = m_threadMgr;

            CComQIPtr<ITfSource> evtCtx = m_context;
            if (FAILED(hr = evtCtx->AdviseSink(IID_ITfContextOwner, this, &m_contextOwnerCookie))) goto Cleanup;

            if (FAILED(hr = m_compositionHandler->initialize(m_clientId, m_context))) goto Cleanup;

            if (FAILED(hr = m_inputProcessor->initialize(m_threadMgr))) goto Cleanup;

            if (FAILED(hr = m_candHandler->initialize(uiElementMgr))) goto Cleanup;
            if (FAILED(hr = m_fullScHandler->initialize(uiElementMgr))) goto Cleanup;

            if (FAILED(hr = m_conversionHander->initialize(m_clientId, compartmentMgr))) goto Cleanup;
            if (FAILED(hr = m_sentenceHander->initialize(m_clientId, compartmentMgr))) goto Cleanup;

            return S_OK;
        Cleanup:
            dispose();
            return hr;
        }

        /**
         * @brief Dispose context
         */
        HRESULT dispose()
        {
            // Prevent creation of new Composition
            CHECK_HR(setIMState(false));

            // Cleanup
            CHECK_HR(m_compositionHandler->dispose());
            CHECK_HR(m_inputProcessor->dispose());
            CHECK_HR(m_candHandler->dispose());
            CHECK_HR(m_fullScHandler->dispose());
            CHECK_HR(m_conversionHander->dispose());
            CHECK_HR(m_sentenceHander->dispose());

            // Context will unadvise all the sinks when pop
            // So we dont need to unadvise the ITfContextOwner sink
            CHECK_HR(m_documentMgr->Pop(TF_POPF_ALL));

            CHECK_HR(m_threadMgr->Deactivate());

            return S_OK;
        }

        /**
         * @brief Terminate the active composition
         */
        HRESULT terminateComposition()
        {
            CComQIPtr<ITfContextOwnerCompositionServices> contextServices = m_context;
            // Pass NULL to terminate all the compositions
            CHECK_HR(contextServices->TerminateComposition(NULL));
            return S_OK;
        }

        /**
         * @brief Set input method state
         *
         * @param enable true to enable the input method, false to disable it
         */
        HRESULT setIMState(bool enable)
        {
            if (enable) { CHECK_HR(m_threadMgr->SetFocus(m_documentMgr)); }
            else {
                // Focus on a document without context to disable input method
                CComPtr<ITfDocumentMgr> documentMgr;
                m_threadMgr->CreateDocumentMgr(&documentMgr);
                CHECK_HR(m_threadMgr->SetFocus(documentMgr));
            }
            m_imState = enable;
            return S_OK;
        }

        /**
         * @brief Get input method(IM) state
         *
         * @param imState returns true if IM is enabled, false otherwise
         */
        HRESULT getIMState(bool* imState)
        {
            *imState = m_imState;
            return S_OK;
        }

        /**
         * @brief This method should be called from the WndProc of the ui-thread, of whom is the creator of this
         * context
         *
         * @param hWnd The window who receives the message
         * @param message can be one of WM_SETFOCUS/WM_KILLFOCUS
         */
        HRESULT onFocusMsg(HWND hWnd, UINT message)
        {
            switch (message) {
                case WM_SETFOCUS: m_hWnd = hWnd; return setIMState(m_imState);
                case WM_KILLFOCUS:
                    if (m_hWnd == hWnd) m_hWnd = NULL;
                    return S_OK;
                default: return E_INVALIDARG;
            }
        }

        /**
         * @brief Get current focused window
         *
         * @param hWnd current focused window, this can be NULL if no window get focused
         */
        HRESULT getFocusedWnd(HWND* hWnd)
        {
            *hWnd = m_hWnd;
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
         * @brief Input method call this method to get the bounding box, in screen coordinates, of the preedit texts,
         * and use which to position its candidate window
         */
        HRESULT STDMETHODCALLTYPE GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override
        {
            m_compositionHandler->m_sigBoundingBox(prc);
            return S_OK;
        }

        /**
         * @brief Return the bounding box, in screen coordinates, of the display surface of the text stream
         */
        HRESULT STDMETHODCALLTYPE GetScreenExt(RECT* prc) override
        {
            // If no window focused, just return nothing
            if (m_hWnd) GetWindowRect(m_hWnd, prc);
            return S_OK;
        }

        /**
         * @brief Obtains the status of a document.
         */
        HRESULT STDMETHODCALLTYPE GetStatus(TF_STATUS* pdcs) override
        {
            // Return 0 means the current context is editable
            pdcs->dwDynamicFlags = 0;
            // Return 0 means current context only support single selection
            pdcs->dwStaticFlags = 0;
            return S_OK;
        }

        /**
         * @brief Return current focused window, can be retrieve by ITfContextView, which can be query interface from
         * ITfContext
         */
        HRESULT STDMETHODCALLTYPE GetWnd(HWND* phwnd) override
        {
            phwnd = &m_hWnd;
            return S_OK;
        }

        /**
         * @brief Our context doesn't support any attributes, just return VT_EMPTY
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