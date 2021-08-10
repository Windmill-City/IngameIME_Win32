#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

extern "C"
{
    typedef unsigned long ConversionMode;
    typedef void (*CallbackConversionMode)(HWND, ConversionMode);
}

namespace libtf
{
    class ConversionModeHandler : public CComObjectRoot, public ITfCompartmentEventSink
    {
    protected:
        CComPtr<ITfCompartmentMgr> m_compartmentMgr;
        CComPtr<ITfCompartment> m_conversionMode;
        DWORD m_conversionModeCookie;
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
        typedef std::function<void(HWND, ConversionMode)> signalConversionMode;
        signalConversionMode sigConversionMode = [](HWND, ConversionMode) {};

        BEGIN_COM_MAP(ConversionModeHandler)
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
            CHECK_HR(m_compartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &m_conversionMode));
            CHECK_HR(m_conversionMode.Advise(this, IID_ITfCompartmentEventSink, &m_conversionModeCookie))
            return S_OK;
        }

        /**
         * @brief Dispose the handler
         * 
         * @return HRESULT
         */
        HRESULT dispose()
        {
            CComQIPtr<ITfSource> evtConversionMode = m_conversionMode;
            return evtConversionMode->UnadviseSink(m_conversionModeCookie);
        }

        /**
         * @brief Set the Conversion Mode of current thread
         * 
         * @param mode mode
         * @return HRESULT 
         */
        HRESULT setConversionMode(ConversionMode mode)
        {
            CComVariant val;
            val.ulVal = mode;
            return m_conversionMode->SetValue(m_clientId, &val);
        }

        /**
         * @brief Handle Conversion Mode change
         */
        HRESULT OnChange(REFGUID rguid) override
        {
            if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
            {
                CComVariant val;
                CHECK_HR(m_conversionMode->GetValue(&val));
                HWND hWnd;
                CHECK_HR(getWnd(hWnd));
                sigConversionMode(hWnd, val.ulVal);
            }
            return S_OK;
        }
    };

    typedef CComObject<ConversionModeHandler> CConversionModeHandler;
}