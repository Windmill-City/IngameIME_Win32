#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

extern "C" {
typedef struct tagInputProcessorActivation
{
    /**
     * @brief The type of this profile. This is one of these values.
     * TF_PROFILETYPE_INPUTPROCESSOR - This is a text service.
     * TF_PROFILETYPE_KEYBOARDLAYOUT - This is a keyboard layout.
     */
    DWORD dwProfileType;
    /**
     * @brief Specifies the language id of the profile.
     */
    LANGID langid;
    /**
     * @brief Specifies the CLSID of the text service.
     * If dwProfileType is TF_PROFILETYPE_KEYBOARDLAYOUT, this is CLSID_NULL.
     */
    CLSID clsid;
    /**
     * @brief Specifies the category of this text service.
     * This category is GUID_TFCAT_TIP_KEYBOARD, GUID_TFCAT_TIP_SPEECH, GUID_TFCAT_TIP_HANDWRITING or something in
     * GUID_TFCAT_CATEGORY_OF_TIP. If dwProfileType is TF_PROFILETYPE_KEYBOARDLAYOUT, this is GUID_NULL.
     */
    GUID catid;
    /**
     * @brief Specifies the GUID to identify the profile.
     * If dwProfileType is TF_PROFILETYPE_KEYBOARDLAYOUT, this is GUID_NULL.
     */
    GUID guidProfile;
    /**
     * @brief Specifies the keyboard layout handle of this profile.
     * If dwProfileType is TF_PROFILETYPE_ INPUTPROCESSOR, this is NULL.
     */
    HKL hkl;
    /**
     * @brief TF_IPSINK_FLAG_ACTIVE - This is on if this profile is activated.
     */
    DWORD dwFlags;

} libtf_InputProcessorActivation_t, libtf_pInputProcessorActivation;
typedef void (*libtf_CallbackInputProcessor)(libtf_InputProcessorActivation_t);
};

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