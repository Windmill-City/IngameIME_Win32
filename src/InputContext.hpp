#pragma once
#include "libtfdef.h"

#include "TfThread.hpp"

#include "CandidateListHandler.hpp"
#include "CompositionHandler.hpp"
#include "ConversionModeHandler.hpp"
#include "FullScreenUIElementHandler.hpp"
#include "SentenceModeHandler.hpp"

#include <memory>
#include <msctf.h>

namespace libtf
{
    class InputContext : public CComObjectRoot, protected ITfContextOwner
    {
    protected:
        /**
         * @brief STA Apartment Thread, where we CoCreate ITfThreadMgr
         */
        std::unique_ptr<TfThread> m_tfThread = std::make_unique<TfThread>();
        CComPtr<ITfThreadMgr> m_threadMgr;
        TfClientId m_clientId;
        CComPtr<ITfDocumentMgr> m_documentMgr;
        CComPtr<ITfContext> m_context;
        DWORD m_contextOwnerCookie;
        HWND m_hWnd = NULL;
        bool m_imState;

    public:
        CComPtr<CCandidateListHandler> m_candHandler = new CCandidateListHandler();
        CComPtr<CFullScreenUIElementHandler> m_fullScHandler = new CFullScreenUIElementHandler();
        CComPtr<CCompositionHandler> m_compositionHandler = new CCompositionHandler();
        CComPtr<CConversionModeHandler> m_conversionHander = new CConversionModeHandler();
        CComPtr<CSentenceModeHandler> m_sentenceHander = new CSentenceModeHandler();

        BEGIN_COM_MAP(InputContext)
        COM_INTERFACE_ENTRY(ITfContextOwner)
        END_COM_MAP()

        /**
         * @brief Initialize InputContext on the calling thread
         * 
         * @return HRESULT 
         */
        HRESULT initialize()
        {
            IStream *streamThreadMgr;
            IStream *streamUIElementMgr;
            CHECK_HR(m_tfThread->enqueue(
                                   [&streamThreadMgr, &streamUIElementMgr]()
                                   {
                                       CComPtr<ITfThreadMgr> threadMgr;
                                       CHECK_HR(threadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER));
                                       CHECK_HR(CoMarshalInterThreadInterfaceInStream(IID_ITfThreadMgr, (IUnknown *)threadMgr.p, &streamThreadMgr));

                                       CComQIPtr<ITfUIElementMgr> uiElementMgr = threadMgr;
                                       CHECK_HR(CoMarshalInterThreadInterfaceInStream(IID_ITfUIElementMgr, (IUnknown *)uiElementMgr.p, &streamUIElementMgr));

                                       return S_OK;
                                   })
                         .get());

            CHECK_HR(CoGetInterfaceAndReleaseStream(streamThreadMgr, IID_ITfThreadMgr, (void **)&m_threadMgr));

            CComPtr<ITfUIElementMgr> uiElementMgr;
            CHECK_HR(CoGetInterfaceAndReleaseStream(streamUIElementMgr, IID_ITfUIElementMgr, (void **)&uiElementMgr));

            CComQIPtr<ITfThreadMgrEx> threadMgrEx = m_threadMgr;
            CHECK_HR(threadMgrEx->ActivateEx(&m_clientId, TF_TMAE_UIELEMENTENABLEDONLY));

            CHECK_HR(setIMState(false));

            CHECK_HR(m_threadMgr->CreateDocumentMgr(&m_documentMgr));

            TfEditCookie ec;
            CHECK_HR(m_documentMgr->CreateContext(m_clientId, 0, (ITfContextOwnerCompositionSink *)m_compositionHandler, &m_context, &ec));
            CHECK_HR(m_documentMgr->Push(m_context));

            HRESULT hr;
            CComQIPtr<ITfCompartmentMgr> compartmentMgr = m_threadMgr;

            CComQIPtr<ITfSource> evtCtx = m_context;
            if (FAILED(hr = evtCtx->AdviseSink(IID_ITfContextOwner, this, &m_contextOwnerCookie))) goto Cleanup;

            if (FAILED(hr = m_compositionHandler->initialize(m_clientId, m_context))) goto Cleanup;

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
         * 
         * @return HRESULT 
         */
        HRESULT dispose()
        {
            //Prevent creation of new Composition
            CHECK_HR(setIMState(false));

            //Cleanup
            CHECK_HR(m_compositionHandler->dispose());
            CHECK_HR(m_candHandler->dispose());
            CHECK_HR(m_fullScHandler->dispose());
            CHECK_HR(m_conversionHander->dispose());
            CHECK_HR(m_sentenceHander->dispose());

            //Context will un-advise all the sinks when pop
            CHECK_HR(m_documentMgr->Pop(TF_POPF_ALL));

            CHECK_HR(m_threadMgr->Deactivate());

            return S_OK;
        }

        /**
         * @brief Terminate all the compositions
         * 
         * @return HRESULT 
         */
        HRESULT terminateComposition()
        {
            CComQIPtr<ITfContextOwnerCompositionServices> contextServices = m_context;
            //Pass NULL to terminate all the compositions
            CHECK_HR(contextServices->TerminateComposition(NULL));
            return S_OK;
        }

        /**
         * @brief Set input method state
         * 
         * @param enable enable?
         * @return HRESULT 
         */
        HRESULT setIMState(bool enable)
        {
            if (enable)
            {
                CHECK_HR(m_threadMgr->SetFocus(m_documentMgr));
            }
            else
            {
                //Focus on a document without context to disable input method
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
         * @return true IM has enabled
         * @return false IM has disabled
         */
        HRESULT getIMState(bool *imState)
        {
            *imState = m_imState;
            return S_OK;
        }

        /**
         * @brief Set current focused window
         * 
         * @param hWnd window who receive WM_SETFOCUS on its message queue
         *             this parameter can be NULL if the context does not have the corresponding handle to the window.
         * @return HRESULT 
         */
        HRESULT setFocus(HWND hWnd)
        {
            m_hWnd = hWnd;
            return setIMState(m_imState);
        }
#pragma region ITfContextOwner
        /**
         * @brief Converts a point in screen coordinates to an application character position.
         * 
         * @param ptScreen Pointer to the POINT structure with the screen coordinates of the point.
         * @param dwFlags Specifies the character position to return based upon the screen coordinates of the point relative to a character bounding box. 
         * By default, the character position returned is the character bounding box containing the screen coordinates of the point.
         * If the point is outside a character's bounding box, the method returns NULL or TF_E_INVALIDPOINT.
         * 
         * If the GXFPF_ROUND_NEAREST flag is specified for this parameter
         * and the screen coordinates of the point are contained in a character bounding box,
         * the character position returned is the bounding edge closest to the screen coordinates of the point.
         * 
         * If the GXFPF_NEAREST flag is specified for this parameter
         * and the screen coordinates of the point are not contained in a character bounding box,
         * the closest character position is returned.
         * 
         * The bit flags can be combined.
         * @param pacp Receives the character position that corresponds to the screen coordinates of the point
         * @return HRESULT
         *         S_OK - The method was successful.
         *         TS_E_INVALIDPOINT - The ptScreen parameter is not within the bounding box of any character.
         *         TS_E_NOLAYOUT - The application has not calculated a text layout.
         * https://docs.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfcontextowner-getacpfrompoint
         */
        HRESULT GetACPFromPoint(const POINT *ptScreen, DWORD dwFlags, LONG *pacp) override
        {
            return TS_E_NOLAYOUT;
        }

        /**
         * @brief returns the bounding box, in screen coordinates, of the text at a specified character position.
         * The caller must have a read-only lock on the document before calling this method.
         * 
         * If the document window is minimized, or if the specified text is not currently visible,
         * the method returns S_OK with the prc parameter set to {0,0,0,0}.
         * 
         * @param acpStart Specifies the starting character position of the text to get in the document.
         * @param acpEnd Specifies the ending character position of the text to get in the document.
         * @param prc Receives the bounding box, in screen coordinates, of the text at the specified character positions.
         * @param pfClipped Receives the Boolean value that specifies if the text in the bounding box has been clipped.
         *                  If this parameter is TRUE, the bounding box contains clipped text
         *                  and does not include the entire requested range of text.
         *                  The bounding box is clipped because of the requested range is not visible.
         * @return HRESULT
         *         S_OK - The method was successful.
         *         TS_E_INVALIDARG - The specified start and end character positions are equal.
         *         TS_E_INVALIDPOS - The range specified by the acpStart and acpEnd parameters
         *                           extends past the end of the document or the top of the document.
         *         TS_E_NOLAYOUT - The application has not calculated a text layout.
         */
        HRESULT GetTextExt(LONG acpStart, LONG acpEnd, RECT *prc, BOOL *pfClipped) override
        {
            //Always return the bounding box of pre edit text, for positioning the Candidate List window
            m_compositionHandler->m_sigBoundingBox(prc);
            return S_OK;
        }

        /**
         * @brief Returns the bounding box, in screen coordinates, of the display surface where the text stream is rendered.
         * 
         * If the text is not currently displayed, for example if the document window is minimized, prc is set to { 0, 0, 0, 0 }.
         * 
         * @param prc Receives the bounding box screen coordinates of the display surface of the document.
         * @return HRESULT 
         */
        HRESULT GetScreenExt(RECT *prc) override
        {
            if (m_hWnd)
                GetWindowRect(m_hWnd, prc);
            return S_OK;
        }

        /**
         * @brief Obtains the status of a document.
         * The document status is returned through the TS_STATUS structure.
         * 
         * @param pdcs TS_STATUS structure
         * @return HRESULT 
         */
        HRESULT GetStatus(TF_STATUS *pdcs) override
        {
            pdcs->dwDynamicFlags = 0;
            pdcs->dwStaticFlags = 0;
            return S_OK;
        }

        /**
         * @brief Returns the handle to a window that corresponds to the current document.
         * 
         * A document might not have a corresponding window handle
         * if the document is in memory but not displayed on the screen
         * or if the document is a windowless control and the control
         * does not know the window handle of the owner of the windowless controls.
         * Callers cannot assume that the phwnd parameter will receive a non-NULL value even if the method is successful.
         * Callers can also receive a NULL value for the phwnd parameter.
         * 
         * @param phwnd Receives a pointer to the handle of the window that corresponds to the current document.
         *              This parameter can be NULL if the document does not have the corresponding handle to the window.
         * @return HRESULT 
         */
        HRESULT GetWnd(HWND *phwnd) override
        {
            phwnd = &m_hWnd;
            return S_OK;
        }

        /**
         * @brief Returns the value of a supported attribute.
         * If the attribute is unsupported, the pvarValue parameter is set to VT_EMPTY.
         * 
         * Context owners using the default text store of the TSF manager can implement a simplified version of attributes with this method. 
         * The supported attributes are application or text service dependent. 
         * For more information about predefined attributes recognized in TSF, see the following topics.
         * https://docs.microsoft.com/en-us/windows/win32/api/msctf/nf-msctf-itfcontextowner-getattribute
         * 
         * @param rguidAttribute Specifies the attribute GUID.
         * @param pvarValue Receives the attribute value. If the attribute is unsupported, this parameter is set to VT_EMPTY.
         * @return HRESULT 
         */
        HRESULT GetAttribute(REFGUID rguidAttribute, VARIANT *pvarValue) override
        {
            pvarValue->vt = VT_EMPTY;
            return S_OK;
        }
#pragma endregion
    };

    typedef CComObjectNoLock<InputContext> CInputContext;
} // namespace libtf