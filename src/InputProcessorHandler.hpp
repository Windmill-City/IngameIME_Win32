#pragma once
#include <algorithm>
#include <list>

#include <atlbase.h>
#include <atlcom.h>

#include <msctf.h>

#include "libtfdef.h"

#include "ICallback.hpp"
#include "IThreadAssociate.hpp"

namespace libtf {
    class InputModeHandler : public CComObjectRoot,
                             public ITfCompartmentEventSink,
                             public IThreadAssociate,
                             public ICallback<const int> {
        CComQIPtr<ITfCompartmentMgr> m_CompartmentMgr;
        CComPtr<ITfCompartment>      m_ConversionMode;
        DWORD                        m_ConversionModeCookie = TF_INVALID_COOKIE;
        TfClientId                   m_ClientId;

      public:
        /**
         * @brief Create handler
         *
         * @param threadMgr ITfThreadMgr
         */
        HRESULT ctor(TfClientId clientId, CComPtr<ITfThreadMgr> threadMgr)
        {
            BEGIN_HRESULT();

            initialCreatorThread();

            m_ClientId       = clientId;
            m_CompartmentMgr = threadMgr;

            CHECK_HR(
                m_CompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &m_ConversionMode));

            CComQIPtr<ITfSource> source = m_ConversionMode;
            CHECK_HR(source->AdviseSink(IID_ITfCompartmentEventSink, this, &m_ConversionModeCookie))
            END_HRESULT();
        }

        /**
         * @brief Free event sink
         *
         * @return HRESULT
         */
        HRESULT dtor()
        {
            BEGIN_HRESULT();

            if (m_ConversionModeCookie == TF_INVALID_COOKIE) return S_OK;

            CComQIPtr<ITfSource> source = m_ConversionMode;
            CHECK_HR(source->UnadviseSink(m_ConversionModeCookie));
            m_ConversionModeCookie = TF_INVALID_COOKIE;

            END_HRESULT();
        }

      private:
        friend class InputProcessorHandler;
        HRESULT getConversionMode(int& mode)
        {
            BEGIN_HRESULT();

            CComVariant val;

            CHECK_HR(m_ConversionMode->GetValue(&val));
            mode = val.intVal;

            END_HRESULT();
        }

        HRESULT setConversionMode(int mode)
        {
            BEGIN_HRESULT();

            CComVariant val;

            val.vt     = VT_I4;
            val.intVal = mode;

            CHECK_HR(m_ConversionMode->SetValue(m_ClientId, &val));

            END_HRESULT();
        }

      public:
        BEGIN_COM_MAP(InputModeHandler)
        COM_INTERFACE_ENTRY(ITfCompartmentEventSink)
        END_COM_MAP()

        /**
         * @brief Handle Conversion Mode change
         */
        HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override
        {
            BEGIN_HRESULT();
            if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION)) {
                int mode;
                CHECK_HR(getConversionMode(mode));
                runCallback(std::move(mode));
            }
            END_HRESULT();
        }
    };
    typedef CComObjectNoLock<InputModeHandler> CInputModeModeHandler;

    class InputProcessor {
        friend bool              operator==(const InputProcessor& processor, const TF_INPUTPROCESSORPROFILE profile);
        TF_INPUTPROCESSORPROFILE m_TFProfile;

      private:
        static std::wstring getLocale(const LANGID langid)
        {
            LCID lcid = MAKELCID(langid, SORT_DEFAULT);

            int  size = GetLocaleInfoW(lcid, LOCALE_SNAME, NULL, 0);
            auto buf  = std::make_unique<wchar_t[]>(size);

            GetLocaleInfoW(lcid, LOCALE_SNAME, buf.get(), size);

            return std::wstring(buf.get(), size);
        }

        static std::wstring getLocaleName(const LANGID langid)
        {
            LCID lcid = MAKELCID(langid, SORT_DEFAULT);

            int  size = GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, NULL, 0);
            auto buf  = std::make_unique<wchar_t[]>(size);

            GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, buf.get(), size);

            return std::wstring(buf.get(), size);
        }

        static std::wstring getInputProcessorName(const TF_INPUTPROCESSORPROFILE profile)
        {
            HRESULT hr;
            auto    result = std::wstring(L"Unknown InputProcessor Name");

            switch (profile.dwProfileType) {
                case TF_PROFILETYPE_INPUTPROCESSOR: {
                    BEGIN_HRESULT_SCOPE();

                    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
                    CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));

                    CComBSTR name;
                    CHECK_HR(inputProcessorProfiles->GetLanguageProfileDescription(
                        profile.clsid, profile.langid, profile.guidProfile, &name));

                    result = std::wstring(name.m_str);

                    END_HRESULT_SCOPE();
                } break;
                case TF_PROFILETYPE_KEYBOARDLAYOUT: {
                    // Open Main Key
                    HKEY layouts;
                    BEGIN_HRESULT_SCOPE();
                    CHECK_ES(RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                           L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
                                           0,
                                           KEY_READ,
                                           &layouts));

                    // the key of the keyboard layout is its langid
                    char layoutKey[9];
                    snprintf(layoutKey, 9, "%08x", profile.langid);

                    // Open SubKey
                    HKEY layout;
                    BEGIN_HRESULT_SCOPE();
                    CHECK_ES(RegOpenKeyExA(layouts, layoutKey, 0, KEY_READ, &layout));

                    // Get data size first
                    DWORD size;
                    CHECK_ES(RegGetValueW(
                        layout, NULL, L"Layout Display Name", RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, NULL, NULL, &size));
                    // Get resource key of the name
                    auto resKey = std::make_unique<wchar_t[]>(size);
                    CHECK_ES(RegGetValueW(layout,
                                          NULL,
                                          L"Layout Display Name",
                                          RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                          NULL,
                                          resKey.get(),
                                          &size));

                    // Get the layout name by resource key
                    wchar_t layoutName[KL_NAMELENGTH];
                    CHECK_HR(SHLoadIndirectString(resKey.get(), layoutName, KL_NAMELENGTH, NULL));

                    result = std::wstring(layoutName);

                    END_HRESULT_SCOPE();
                    RegCloseKey(layout);

                    END_HRESULT_SCOPE();
                    RegCloseKey(layouts);
                } break;
            }
            return result;
        }

      private:
        friend class InputProcessorHandler;
        /**
         * @brief Get InputModes from conversion mode
         *
         * @param conversionMode conversion mode
         * @return std::list<std::wstring> InputModes
         */
        virtual std::list<std::wstring> getInputModes(const int conversionMode) const = 0;
        /**
         * @brief Apply new input mode to the InputProcessor
         *
         * @param inputModes old modes
         * @param newMode new mode
         * @return std::list<std::wstring> new modes
         */
        virtual std::list<std::wstring> applyNewInputMode(const std::list<std::wstring> inputModes,
                                                          const std::wstring            newMode) const = 0;
        /**
         * @brief Get conversion mode from InputModes
         *
         * @param inputModes InputMode to convert
         * @return int conversion mode
         */
        virtual int getConversionMode(const std::list<std::wstring> inputModes) const = 0;

      public:
        /**
         * @brief Type of the InputProcessor
         */
        libtf_InputProcessorType_t m_Type;
        /**
         * @brief Locale string of the InputProcessor
         */
        std::wstring m_Locale;
        /**
         * @brief Localized name of the locale
         */
        std::wstring m_LocaleName;
        /**
         * @brief Localized name of the InputProcessor
         */
        std::wstring m_InputProcessorName;

      protected:
        InputProcessor(const TF_INPUTPROCESSORPROFILE profile)
        {
            m_TFProfile = profile;
            m_Type = profile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT ? libtf_KeyboardLayout : libtf_TextService;
            m_Locale             = getLocale(m_TFProfile.langid);
            m_LocaleName         = getLocaleName(m_TFProfile.langid);
            m_InputProcessorName = getInputProcessorName(m_TFProfile);
        }

      public:
        static std::shared_ptr<const InputProcessor> create(const TF_INPUTPROCESSORPROFILE profile);
        /**
         * @brief Get InputProcessors available
         *
         * @return Empty list if the calling thread is not a UI Thread
         */
        static std::list<std::shared_ptr<const InputProcessor>> getInputProcessors()
        {
            std::list<std::shared_ptr<const InputProcessor>> result;

            HRESULT hr;
            BEGIN_HRESULT_SCOPE();

            CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
            CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

            CComPtr<IEnumTfInputProcessorProfiles> enumProfiles;
            // Pass 0 to langid to enum all profiles
            CHECK_HR(inputProcessorMgr->EnumProfiles(0, &enumProfiles));

            TF_INPUTPROCESSORPROFILE profile[1];
            while (true) {
                ULONG fetch;
                CHECK_HR(enumProfiles->Next(1, profile, &fetch));

                // No more
                if (fetch == 0) break;

                // InputProcessor not enabled can't be activated
                if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED)) continue;

                result.push_back(create(profile[0]));
            }
            END_HRESULT_SCOPE();

            return result;
        }

      public:
        bool operator==(const InputProcessor& processor) const
        {
            if (m_Type != processor.m_Type) return false;
            if (m_Type == libtf_KeyboardLayout) { return m_TFProfile.hkl == processor.m_TFProfile.hkl; }
            else {
                return IsEqualCLSID(m_TFProfile.clsid, processor.m_TFProfile.clsid);
            }
        }

      public:
        /**
         * @brief Set active InputProcessor for calling thread
         *
         * @return UI_E_WRONG_THREAD if the calling thread is not a UI Thread
         */
        HRESULT setActivated() const
        {
            BEGIN_HRESULT();

            if (!IsGUIThread(false)) return UI_E_WRONG_THREAD;

            CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
            CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

            CHECK_HR(inputProcessorMgr->ActivateProfile(m_TFProfile.dwProfileType,
                                                        m_TFProfile.langid,
                                                        m_TFProfile.clsid,
                                                        m_TFProfile.guidProfile,
                                                        m_TFProfile.hkl,
                                                        TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE));

            END_HRESULT();
        }
    };
    bool operator==(const InputProcessor& processor, const TF_INPUTPROCESSORPROFILE profile)
    {
        if (processor.m_TFProfile.dwProfileType == profile.dwProfileType) {
            if (profile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT)
                return processor.m_TFProfile.hkl == profile.hkl;
            else
                return IsEqualCLSID(processor.m_TFProfile.clsid, profile.clsid);
        }
        return false;
    }
    bool operator!=(const InputProcessor& processor, const TF_INPUTPROCESSORPROFILE profile)
    {
        return !(processor == profile);
    }

    class KeyboardLayout : public InputProcessor {
      public:
        KeyboardLayout(TF_INPUTPROCESSORPROFILE profile) : InputProcessor(profile) {}

        virtual std::list<std::wstring> getInputModes(const int conversionMode) const override
        {
            return std::list<std::wstring>(1, std::wstring(L"AlphaNumeric"));
        }

        virtual std::list<std::wstring> applyNewInputMode(const std::list<std::wstring> inputModes,
                                                          const std::wstring            newMode) const override
        {
            return std::list<std::wstring>(1, std::wstring(L"AlphaNumeric"));
        }

        virtual int getConversionMode(const std::list<std::wstring> inputModes) const override
        {
            // AlphaNumeric
            return 0;
        }
    };

    class TextService : public InputProcessor {
      public:
        TextService(TF_INPUTPROCESSORPROFILE profile) : InputProcessor(profile) {}

        virtual std::list<std::wstring> getInputModes(const int conversionMode) const override
        {
            auto result = std::list<std::wstring>();

            if (conversionMode & TF_CONVERSIONMODE_NATIVE) { result.push_back(L"Native"); }
            else {
                result.push_back(L"AlphaNumeric");
            }

            if (conversionMode & TF_CONVERSIONMODE_FULLSHAPE) { result.push_back(L"FullShape"); }
            else {
                result.push_back(L"HalfShape");
            }

            return result;
        }

        virtual std::list<std::wstring> applyNewInputMode(const std::list<std::wstring> inputModes,
                                                          const std::wstring            newMode) const override
        {
            auto result = std::list<std::wstring>(inputModes);

            if (newMode == L"AlphaNumeric" || newMode == L"Native") {
                result.remove_if([](auto&& oldMode) { return oldMode == L"AlphaNumeric" || oldMode == L"Native"; });
            }

            if (newMode == L"HalfShape" || newMode == L"FullShape") {
                result.remove_if([](auto&& oldMode) { return oldMode == L"HalfShape" || oldMode == L"FullShape"; });
            }

            result.push_back(newMode);

            return result;
        }

        virtual int getConversionMode(const std::list<std::wstring> inputModes) const override
        {
            int result = 0;
            for (auto&& mode : inputModes) {
                if (mode == L"Native") { result |= TF_CONVERSIONMODE_NATIVE; }
                if (mode == L"FullShape") { result |= TF_CONVERSIONMODE_FULLSHAPE; }
            }

            return result;
        }
    };

    class TextService_Japan : public TextService {
      public:
        TextService_Japan(TF_INPUTPROCESSORPROFILE profile) : TextService(profile) {}

        virtual std::list<std::wstring> getInputModes(const int conversionMode) const override
        {
            auto result = TextService::getInputModes(conversionMode);

            if (conversionMode & TF_CONVERSIONMODE_KATAKANA) { result.push_back(L"Katakana"); }
            else if (conversionMode & TF_CONVERSIONMODE_ROMAN) {
                result.push_back(L"Roman");
            }
            else {
                result.push_back(L"Hiragana");
            }

            return result;
        }

        virtual std::list<std::wstring> applyNewInputMode(const std::list<std::wstring> inputModes,
                                                          const std::wstring            newMode) const override
        {
            auto result = TextService::applyNewInputMode(inputModes, newMode);

            if (newMode == L"Hiragana" || newMode == L"Katakana" || newMode == L"Roman") {
                result.remove_if([](auto&& oldMode) {
                    return oldMode == L"Hiragana" || oldMode == L"Katakana" || oldMode == L"Roman";
                });
            }

            result.push_back(newMode);

            return result;
        }

        virtual int getConversionMode(const std::list<std::wstring> inputModes) const override
        {
            int result = TextService::getConversionMode(inputModes);
            for (auto&& mode : inputModes) {
                if (mode == L"Katakana") { result |= TF_CONVERSIONMODE_KATAKANA; }
                if (mode == L"Roman") { result |= TF_CONVERSIONMODE_ROMAN; }
            }

            return result;
        }
    };

    std::shared_ptr<const InputProcessor> InputProcessor::create(const TF_INPUTPROCESSORPROFILE profile)
    {
        // Keep the ref stable
        static std::list<std::shared_ptr<const InputProcessor>> InputProcessors;

        std::shared_ptr<const InputProcessor> result;

        auto iter = std::find_if(
            InputProcessors.begin(), InputProcessors.end(), [profile](auto&& it) { return *it == profile; });

        // Not exist, create new
        if (iter == InputProcessors.end()) {
            if (profile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT) {
                result = std::make_shared<KeyboardLayout>(profile);
            }
            else if (StrCmpW(getLocale(profile.langid).c_str(), L"ja") == 0) {
                result = std::make_shared<TextService_Japan>(profile);
            }
            else {
                result = std::make_shared<TextService>(profile);
            }
            InputProcessors.push_back(result);
        }
        else
            result = *iter;

        return result;
    }

    class InputProcessorContext {
      public:
        /**
         * @brief Active input processor
         */
        std::shared_ptr<const InputProcessor> m_InputProcessor;
        /**
         * @brief Size of m_InputModes
         */
        uint32_t m_InputModeSize;
        /**
         * @brief InputModes of the InputProcessor
         *
         * InputProcessor have different InputModes at different states
         * eg: KeyboardLayout, which has only one state -> 'AlphaNumeric'
         * eg: TextService, in East-Asia, has the following state:
         * {Native, AlphaNumeric}, {HalfShape, FullShape}
         * TextService, in Japan, has extra state:
         * {Hiragana, Katakana, Roman}
         * In the brace are mutually exclusive InputModes
         */
        std::list<std::wstring> m_InputModes;

      public:
        InputProcessorContext(std::shared_ptr<const InputProcessor> processor, std::list<std::wstring> inputModes)
        {
            m_InputProcessor = processor;
            m_InputModes     = inputModes;
            m_InputModeSize  = (uint32_t)m_InputModes.size();
        }
    };

    class InputProcessorHandler
        : public CComObjectRoot,
          public ITfInputProcessorProfileActivationSink,
          public IThreadAssociate,
          public ICallback<const libtf_InputProcessorState_t, std::shared_ptr<const InputProcessorContext>> {
        CComPtr<ITfThreadMgr>          m_ThreadMgr;
        DWORD                          m_InputProcessorActivationSinkCookie = TF_INVALID_COOKIE;
        CComPtr<CInputModeModeHandler> m_InputModeHandler                   = new CInputModeModeHandler();

      public:
        std::shared_ptr<const InputProcessorContext> m_Context;

      public:
        /**
         * @brief Create handler
         *
         * @param  threadMgr ITfThreadMgr
         * @return HRESULT
         */
        HRESULT ctor(TfClientId clientId, CComPtr<ITfThreadMgr> threadMgr)
        {
            BEGIN_HRESULT();

            m_ThreadMgr = threadMgr;
            initialCreatorThread();

            CHECK_HR(m_InputModeHandler->ctor(clientId, threadMgr));

#pragma region Initial InputProcessor Context
            CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
            CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

            TF_INPUTPROCESSORPROFILE profile;
            CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile));

            int mode;
            CHECK_HR(m_InputModeHandler->getConversionMode(mode));

            auto processor  = InputProcessor::create(profile);
            auto inputModes = processor->getInputModes(mode);

            auto ctx  = std::make_shared<InputProcessorContext>(processor, inputModes);
            m_Context = ctx;
#pragma endregion

            CComQIPtr<ITfSource> source = threadMgr;
            CHECK_HR(source->AdviseSink(
                IID_ITfInputProcessorProfileActivationSink, this, &m_InputProcessorActivationSinkCookie));

#pragma region InputMode Callback
            m_InputModeHandler->setCallback([&](int mode) {
                auto inputModes = m_Context->m_InputProcessor->getInputModes(mode);
                auto ctx        = std::make_shared<InputProcessorContext>(m_Context->m_InputProcessor, inputModes);
                m_Context       = ctx;

                runCallback(libtf_InputProcessorInputModeUpdate, std::move(ctx));
            });
#pragma endregion

            END_HRESULT();
        }

        /**
         * @brief Free event sink
         *
         * @return HRESULT
         */
        HRESULT dtor()
        {
            BEGIN_HRESULT();

            CHECK_HR(m_InputModeHandler->dtor());

            if (m_InputProcessorActivationSinkCookie == TF_INVALID_COOKIE) return S_OK;

            CComQIPtr<ITfSource> source = m_ThreadMgr;
            CHECK_HR(source->UnadviseSink(m_InputProcessorActivationSinkCookie));
            m_InputProcessorActivationSinkCookie = TF_INVALID_COOKIE;

            END_HRESULT();
        }

      public:
        BEGIN_COM_MAP(InputProcessorHandler)
        COM_INTERFACE_ENTRY(ITfInputProcessorProfileActivationSink)
        END_COM_MAP()

        /**
         * @brief Apply new InputMode to active InputProcessor
         *
         * @param newMode New InputMode
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
         */
        HRESULT applyInputMode(const std::wstring newMode)
        {
            BEGIN_HRESULT();
            CHECK_HR(assertCreatorThread());

            auto newModes       = m_Context->m_InputProcessor->applyNewInputMode(m_Context->m_InputModes, newMode);
            auto conversionMode = m_Context->m_InputProcessor->getConversionMode(newModes);

            CHECK_HR(m_InputModeHandler->setConversionMode(conversionMode));

            END_HRESULT();
        }

        HRESULT STDMETHODCALLTYPE OnActivated(DWORD    dwProfileType,
                                              LANGID   langid,
                                              REFCLSID clsid,
                                              REFGUID  catid,
                                              REFGUID  guidProfile,
                                              HKL      hkl,
                                              DWORD    dwFlags)
        {
            BEGIN_HRESULT();
            // Only notify active inputprocessor
            if (!(dwFlags & TF_TMF_ACTIVATED)) return S_OK;

            TF_INPUTPROCESSORPROFILE profile;
            profile.dwProfileType = dwProfileType;
            profile.langid        = langid;
            profile.clsid         = clsid;
            profile.catid         = catid;
            profile.guidProfile   = guidProfile;
            profile.hkl           = hkl;

            int mode;
            CHECK_HR(m_InputModeHandler->getConversionMode(mode));

            auto processor  = InputProcessor::create(profile);
            auto inputModes = processor->getInputModes(mode);

            auto ctx  = std::make_shared<InputProcessorContext>(processor, inputModes);
            m_Context = ctx;

            runCallback(libtf_InputProcessorFullUpdate, std::move(ctx));

            END_HRESULT();
        }
    };
    typedef CComObjectNoLock<InputProcessorHandler> CInputProcessorHandler;
}// namespace libtf