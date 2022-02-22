#pragma once
#include <algorithm>
#include <list>
#include <map>

#include <Shlwapi.h>
#include <msctf.h>

#include "ComBSTR.hpp"
#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "IngameIME.hpp"
#include "InputProcessor.hpp"
#include "TfFunction.hpp"

#pragma comment(lib, "shlwapi.lib")

namespace libtf {
    class InputProcessorImpl : public IngameIME::InputProcessor {
      protected:
        struct InternalLocale : public IngameIME::Locale
        {
          protected:
            static std::wstring getLocaleString(const LANGID langid)
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

          public:
            InternalLocale(const LANGID langId) noexcept
            {
                locale = getLocaleString(langId);
                name   = getLocaleName(langId);
            }

          public:
            static std::shared_ptr<const InternalLocale> getLocale(const LANGID langId)
            {
                static std::map<LANGID, std::weak_ptr<const InternalLocale>> weakRefs;

                auto iter = weakRefs.find(langId);

                std::shared_ptr<const InternalLocale> locale;

                // Create new locale if not exist or expired
                if (iter == weakRefs.end() || !(locale = (*iter).second.lock())) {
                    locale           = std::make_shared<InternalLocale>(langId);
                    weakRefs[langId] = locale;
                }

                // Clear unref items
                for (auto it = weakRefs.cbegin(); it != weakRefs.cend();) {
                    if ((*it).second.expired())
                        it = weakRefs.erase(it);
                    else
                        it++;
                }

                return locale;
            }
        };

      protected:
        static std::wstring getInputProcessorName(const TF_INPUTPROCESSORPROFILE profile)
        {
            auto result = std::wstring(L"[unknown]");

            switch (profile.dwProfileType) {
                case TF_PROFILETYPE_INPUTPROCESSOR: {
                    COM_HR_BEGIN(S_OK);

                    ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
                    CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));

                    ComBSTR name;
                    CHECK_HR(inputProcessorProfiles->GetLanguageProfileDescription(
                        profile.clsid, profile.langid, profile.guidProfile, &name));

                    result = std::wstring(name.bstr);

                    COM_HR_END();
                } break;
                case TF_PROFILETYPE_KEYBOARDLAYOUT: {
                    // Open Main Key
                    HKEY layouts;
                    if (ERROR_SUCCESS != RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                                       L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
                                                       0,
                                                       KEY_READ,
                                                       &layouts))
                        break;

                    // the key of the keyboard layout is its langid
                    char layoutKey[9];
                    snprintf(layoutKey, 9, "%08x", profile.langid);

                    // Open SubKey
                    HKEY layout;
                    if (ERROR_SUCCESS == RegOpenKeyExA(layouts, layoutKey, 0, KEY_READ, &layout)) {
                        // Get data size first
                        DWORD size;
                        if (ERROR_SUCCESS == RegGetValueW(layout,
                                                          NULL,
                                                          L"Layout Display Name",
                                                          RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                                          NULL,
                                                          NULL,
                                                          &size)) {
                            // Get resource key of the name
                            auto resKey = std::make_unique<wchar_t[]>(size);
                            if (ERROR_SUCCESS == RegGetValueW(layout,
                                                              NULL,
                                                              L"Layout Display Name",
                                                              RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                                              NULL,
                                                              resKey.get(),
                                                              &size)) {
                                // Get the layout name by resource key
                                wchar_t layoutName[KL_NAMELENGTH];
                                if (SUCCEEDED(SHLoadIndirectString(resKey.get(), layoutName, KL_NAMELENGTH, NULL))) {
                                    result = std::wstring(layoutName);
                                }
                            }
                        }
                        RegCloseKey(layout);
                    }
                    RegCloseKey(layouts);
                } break;
            }
            return result;
        }

      protected:
        TF_INPUTPROCESSORPROFILE profile;

      public:
        /**
         * @brief If this is a Japanese InputProcessor
         *
         */
        bool isJap;

      public:
        InputProcessorImpl(TF_INPUTPROCESSORPROFILE profile) : profile(profile)
        {
            type   = profile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT ?
                         IngameIME::InputProcessorType::KeyboardLayout :
                         IngameIME::InputProcessorType::TextService;
            name   = getInputProcessorName(profile);
            locale = InternalLocale::getLocale(profile.langid);
            isJap  = locale->locale.compare(0, 2, L"ja") == 0;
        }

      public:
        static std::shared_ptr<const InputProcessorImpl> getInputProcessor(TF_INPUTPROCESSORPROFILE profile)
        {
            struct CompareCLSID
            {
                bool operator()(const CLSID& s1, const CLSID& s2) const
                {
                    return memcmp(&s1, &s2, sizeof(CLSID)) < 0;
                }
            };

            static std::map<CLSID, std::weak_ptr<const InputProcessorImpl>, CompareCLSID> weakRefs;

            auto iter = weakRefs.find(profile.clsid);

            std::shared_ptr<const InputProcessorImpl> proc;

            // Create new proc if not exist or expired
            if (iter == weakRefs.end() || !(proc = (*iter).second.lock())) {
                proc                    = std::make_shared<InputProcessorImpl>(profile);
                weakRefs[profile.clsid] = proc;
            }

            // Clear unref items
            for (auto it = weakRefs.cbegin(); it != weakRefs.cend();) {
                if ((*it).second.expired())
                    it = weakRefs.erase(it);
                else
                    it++;
            }

            return proc;
        }

      public:
        virtual void setActivated() const override
        {
            COM_HR_BEGIN(S_OK);

            if (!IsGUIThread(false)) THR_HR(UI_E_WRONG_THREAD);

            ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
            ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
                                                                    inputProcessorProfiles);

            CHECK_HR(inputProcessorMgr->ActivateProfile(profile.dwProfileType,
                                                        profile.langid,
                                                        profile.clsid,
                                                        profile.guidProfile,
                                                        profile.hkl,
                                                        TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE));

            COM_HR_END();
            COM_HR_THR();
        }
    };

    class InputProcessorHandler : public ComObjectBase,
                                  public ITfInputProcessorProfileActivationSink,
                                  public ITfCompartmentEventSink {
      public:
        ComQIPtr<ITfCompartmentMgr> compMgr{IID_ITfCompartmentMgr};
        ComPtr<ITfCompartment>      mode;

      public:
        std::shared_ptr<const InputProcessorImpl> activeProc;

      public:
        InputProcessorHandler()
        {
            COM_HR_BEGIN(S_OK);

            ComPtr<ITfThreadMgr> threadMgr;
            CHECK_HR(getThreadMgr(&threadMgr));
            compMgr = threadMgr;

            ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
            ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
                                                                    inputProcessorProfiles);

            TF_INPUTPROCESSORPROFILE profile;
            CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile));
            activeProc = InputProcessorImpl::getInputProcessor(profile);

            CHECK_HR(compMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &mode));

            COM_HR_END();
            COM_HR_THR();
        }

      protected:
        std::list<std::wstring> getInputModes()
        {
            VARIANT var;
            mode->GetValue(&var);

            std::list<std::wstring> modes;
            if (activeProc->type == IngameIME::InputProcessorType::KeyboardLayout)
                modes.push_back(L"AlphaNumeric");
            else {
                if (var.intVal & TF_CONVERSIONMODE_NATIVE)
                    modes.push_back(L"Native");
                else
                    modes.push_back(L"AlphaNumeric");

                if (var.intVal & TF_CONVERSIONMODE_FULLSHAPE)
                    modes.push_back(L"FullShape");
                else
                    modes.push_back(L"HalfShape");

                if (activeProc->isJap)
                    if (var.intVal & TF_CONVERSIONMODE_KATAKANA)
                        modes.push_back(L"Katakana");
                    else
                        modes.push_back(L"Hiragana");
            }

            return modes;
        }

      public:
        std::unique_ptr<IngameIME::InputProcessorContext> getCtx()
        {
            auto ctx            = std::make_unique<IngameIME::InputProcessorContext>();
            ctx->inputProcessor = activeProc;
            ctx->inputModes     = getInputModes();

            return ctx;
        }

      public:
        COM_DEF_BEGIN();
        COM_DEF_INF(ITfInputProcessorProfileActivationSink);
        COM_DEF_INF(ITfCompartmentEventSink);
        COM_DEF_END();

        HRESULT STDMETHODCALLTYPE OnActivated(DWORD    dwProfileType,
                                              LANGID   langid,
                                              REFCLSID clsid,
                                              REFGUID  catid,
                                              REFGUID  guidProfile,
                                              HKL      hkl,
                                              DWORD    dwFlags)
        {
            COM_HR_BEGIN(S_OK);

            // Only notify active inputprocessor
            if (!(dwFlags & TF_IPSINK_FLAG_ACTIVE)) return S_OK;

            TF_INPUTPROCESSORPROFILE profile;
            profile.dwProfileType = dwProfileType;
            profile.langid        = langid;
            profile.clsid         = clsid;
            profile.catid         = catid;
            profile.guidProfile   = guidProfile;
            profile.hkl           = hkl;

            activeProc = InputProcessorImpl::getInputProcessor(profile);

            IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::FullUpdate, *getCtx());

            COM_HR_END();
            COM_HR_RET();
        }

        HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override
        {
            COM_HR_BEGIN(S_OK);

            if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
                IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::FullUpdate, *getCtx());

            COM_HR_END();
            COM_HR_RET();
        }
    };
}// namespace libtf