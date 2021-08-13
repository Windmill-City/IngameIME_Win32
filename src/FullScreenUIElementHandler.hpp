#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

namespace libtf {
    class FullScreenUIElementHandler : public CComObjectRoot, public ITfUIElementSink {
      protected:
        CComPtr<ITfUIElementMgr> m_uiElementMgr;
        DWORD                    m_uiSinkCookie = TF_INVALID_COOKIE;

      public:
        /**
         * @brief Set if input method show hide all its windows
         * Set to true in full screen mode
         */
        bool m_isFullScreen = false;

        BEGIN_COM_MAP(FullScreenUIElementHandler)
        COM_INTERFACE_ENTRY(ITfUIElementSink)
        END_COM_MAP()

        /**
         * @brief Initialize the handler
         *
         * @param uiElementMgr Query interface from ITfThreadMgr
         * @return HRESULT if initialize succeed
         */
        HRESULT initialize(CComPtr<ITfUIElementMgr> uiElementMgr)
        {
            m_uiElementMgr           = uiElementMgr;
            CComQIPtr<ITfSource> evt = m_uiElementMgr;
            return evt->AdviseSink(IID_ITfUIElementSink, this, &m_uiSinkCookie);
        }

        /**
         * @brief Dispose the handler
         *
         * @return HRESULT if dispose succeed
         */
        HRESULT dispose()
        {
            CComQIPtr<ITfSource> evtSource = m_uiElementMgr;
            return evtSource->UnadviseSink(m_uiSinkCookie);
        }

        /**
         * @brief Hide all the window of input method in full screen mode
         *
         * @param pbShow should show?
         */
        HRESULT BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override
        {
            *pbShow = !m_isFullScreen;
            return S_OK;
        }

        /**
         * @brief Do nothing
         */
        HRESULT UpdateUIElement(DWORD dwUIElementId) override
        {
            return S_OK;
        }

        /**
         * @brief Do nothing
         */
        HRESULT EndUIElement(DWORD dwUIElementId) override
        {
            return S_OK;
        }
    };

    typedef CComObjectNoLock<FullScreenUIElementHandler> CFullScreenUIElementHandler;
}// namespace libtf