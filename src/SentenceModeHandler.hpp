#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

extern "C"
{
    typedef unsigned long libtf_SentenceMode;
    typedef void (*libtf_CallbackSentenceMode)(libtf_SentenceMode);
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

    public:
        typedef std::function<void(libtf_SentenceMode)> signalSentenceMode;
        signalSentenceMode sigSentenceMode = [](libtf_SentenceMode) {};

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
        HRESULT setSentenceMode(libtf_SentenceMode mode)
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
                sigSentenceMode(val.ulVal);
            }
            return S_OK;
        }
    };

    typedef CComObjectNoLock<SentenceModeHandler> CSentenceModeHandler;
}