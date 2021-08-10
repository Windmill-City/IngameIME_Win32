#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

extern "C"
{
    typedef unsigned long SentenceMode;
    typedef void (*CallbackSentenceMode)(HWND, SentenceMode);
}

namespace libtf
{
    class SentenceModeHandler : public CComObjectRoot, public ITfCompartmentEventSink
    {
    protected:
        CComPtr<ITfCompartmentMgr> m_compartmentMgr;
        CComPtr<ITfCompartment> m_sentenceMode;
        DWORD m_sentenceModeCookie;
        TfClientId m_clientId;

        /**
         * @brief Get which window is active window on current thread
         * 
         * @param hWnd receive hWnd
         * @return HRESULT 
         */
        HRESULT getWnd(HWND &hWnd)
        {
            CComQIPtr<ITfThreadMgr> threadMgr = m_compartmentMgr;
            CComPtr<ITfDocumentMgr> documentMgr;
            CHECK_HR(threadMgr->GetFocus(&documentMgr));
            CComPtr<ITfContext> context;
            CHECK_HR(documentMgr->GetTop(&context));
            CComQIPtr<ITfContextOwner> contextOwner = context;
            CHECK_HR(contextOwner->GetWnd(&hWnd));
            return S_OK;
        }

    public:
        typedef std::function<void(HWND, SentenceMode)> signalSentenceMode;
        signalSentenceMode sigSentenceMode = [](HWND, SentenceMode) {};

        BEGIN_COM_MAP(SentenceModeHandler)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
        END_COM_MAP()

        /**
         * @brief Initialize handler
         * 
         * @param compartmentMgr Query interface from ITfThreadMgr
         * @return HRESULT
         */
        HRESULT initialize(TfClientId clientId, CComPtr<ITfCompartmentMgr> compartmentMgr)
        {
            m_clientId = clientId;
            m_compartmentMgr = compartmentMgr;
            CHECK_HR(m_compartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE, &m_sentenceMode));
            CComQIPtr<ITfSource> evt = m_sentenceMode;
            CHECK_HR(evt->AdviseSink(IID_ITfCompartmentEventSink, this, &m_sentenceModeCookie))
            return S_OK;
        }

        /**
         * @brief Dispose the handler
         * 
         * @return HRESULT
         */
        HRESULT dispose()
        {
            CComQIPtr<ITfSource> evtSentenceMode = m_sentenceMode;
            return evtSentenceMode->UnadviseSink(m_sentenceModeCookie);
        }

        /**
         * @brief Set the Sentence Mode of current thread
         * 
         * @param mode mode
         * @return HRESULT 
         */
        HRESULT setSentenceMode(SentenceMode mode)
        {
            CComVariant val;
            val.ulVal = mode;
            return m_sentenceMode->SetValue(m_clientId, &val);
        }

        /**
         * @brief Handle Sentence Mode change
         */
        HRESULT OnChange(REFGUID rguid) override
        {
            if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_SENTENCE))
            {
                CComVariant val;
                CHECK_HR(m_sentenceMode->GetValue(&val));
                HWND hWnd;
                CHECK_HR(getWnd(hWnd));
                sigSentenceMode(hWnd, val.ulVal);
            }
            return S_OK;
        }
    };

    typedef CComObjectNoLock<SentenceModeHandler> CSentenceModeHandler;
}