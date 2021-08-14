#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

namespace libtf {
    class InputProcessorHandler : public CComObjectRoot, public ITfInputProcessorProfileActivationSink {
      protected:
        CComPtr<ITfThreadMgr> m_threadMgr;
        DWORD                 m_inputProcessorActivationSinkCookie;

      public:
        /**
         * @brief Callback when Input Processor Activated or Deactivated
         */
        typedef std::function<void(libtf_InputProcessorActivation_t)> signalInputProcessor;
        signalInputProcessor sigInputProcessor = [](libtf_InputProcessorActivation_t) {};

        BEGIN_COM_MAP(InputProcessorHandler)
        COM_INTERFACE_ENTRY(ITfInputProcessorProfileActivationSink)
        END_COM_MAP()

        /**
         * @brief Initialize handler
         *
         * @param  threadMgr ITfThreadMgr
         * @return HRESULT
         */
        HRESULT initialize(CComPtr<ITfThreadMgr> threadMgr)
        {
            m_threadMgr                    = threadMgr;
            CComQIPtr<ITfSource> evtSource = m_threadMgr;
            return evtSource->AdviseSink(
                IID_ITfInputProcessorProfileActivationSink, this, &m_inputProcessorActivationSinkCookie);
        }

        /**
         * @brief Dispose the handler
         *
         * @return HRESULT
         */
        HRESULT dispose()
        {
            CComQIPtr<ITfSource> evtSource = m_threadMgr;
            return evtSource->UnadviseSink(m_inputProcessorActivationSinkCookie);
        }

#pragma region ITfInputProcessorActivationSink
        /**
         * @brief Pack params into a struct
         */
        HRESULT STDMETHODCALLTYPE OnActivated(DWORD    dwProfileType,
                                              LANGID   langid,
                                              REFCLSID clsid,
                                              REFGUID  catid,
                                              REFGUID  guidProfile,
                                              HKL      hkl,
                                              DWORD    dwFlags)
        {
            sigInputProcessor({dwProfileType, langid, clsid, catid, guidProfile, hkl, dwFlags});

            return S_OK;
        }
#pragma endregion
    };

    typedef CComObjectNoLock<InputProcessorHandler> CInputProcessorHandler;
}// namespace libtf