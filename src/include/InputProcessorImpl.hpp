#pragma once
#include <algorithm>
#include <list>
#include <map>

#include <Windows.h>

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <imm.h>
#pragma comment(lib, "imm32.lib")

#include <WinUser.h>
#include <msctf.h>

#include "ComBSTR.hpp"
#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "IngameIME.hpp"
#include "InputProcessor.hpp"
#include "TfFunction.hpp"

namespace IngameIME {
    struct InternalLocale : public IngameIME::Locale
    {
      protected:
        static std::map<LANGID, std::weak_ptr<const InternalLocale>> weakRefs;

      protected:
        const LANGID langId;

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
        InternalLocale(const LANGID langId) noexcept : langId(langId)
        {
            locale = getLocaleString(langId);
            name   = getLocaleName(langId);
        }

        ~InternalLocale() noexcept
        {
            weakRefs.erase(langId);
        }

      public:
        static std::shared_ptr<const InternalLocale> getLocale(const LANGID langId)
        {
            auto iter = weakRefs.find(langId);

            std::shared_ptr<const InternalLocale> locale;

            // Create new locale if not exist or expired
            if (iter == weakRefs.end() || !(locale = (*iter).second.lock())) {
                locale           = std::make_shared<InternalLocale>(langId);
                weakRefs[langId] = locale;
            }

            return locale;
        }
    };
    std::map<LANGID, std::weak_ptr<const InternalLocale>> InternalLocale::weakRefs = {};

    std::wstring getKeyboardLayoutName(LANGID langId)
    {
        auto result = std::wstring(L"[unknown]");

        HKEY layouts;
        if (ERROR_SUCCESS ==
            RegOpenKeyExW(
                HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", 0, KEY_READ, &layouts)) {
            // the key of the keyboard layout is its langid
            char layoutKey[9];
            snprintf(layoutKey, 9, "%08x", langId);

            // Open SubKey
            HKEY layout;
            if (ERROR_SUCCESS == RegOpenKeyExA(layouts, layoutKey, 0, KEY_READ, &layout)) {
                // Get data size first
                DWORD size;
                if (ERROR_SUCCESS ==
                    RegGetValueW(
                        layout, NULL, L"Layout Display Name", RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, NULL, NULL, &size)) {
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
        }

        return result;
    }
}// namespace IngameIME

namespace libtf {
    class InputProcessorImpl : public IngameIME::InputProcessor {
      protected:
        struct CompareProfile
        {
            bool operator()(const TF_INPUTPROCESSORPROFILE& s1, const TF_INPUTPROCESSORPROFILE& s2) const
            {
                if (s1.dwProfileType == s2.dwProfileType)
                    if (s1.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT)
                        return s1.hkl < s2.hkl;
                    else
                        return memcmp(&s1.clsid, &s2.clsid, sizeof(CLSID)) < 0;
                else
                    return s1.dwProfileType < s2.dwProfileType;
            }
        };

        static std::map<TF_INPUTPROCESSORPROFILE, std::weak_ptr<const InputProcessorImpl>, CompareProfile> weakRefs;

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
                    result = IngameIME::getKeyboardLayoutName(profile.langid);
                } break;
            }
            return result;
        }

      protected:
        const TF_INPUTPROCESSORPROFILE profile;

      public:
        /**
         * @brief If this is a Japanese InputProcessor
         *
         */
        bool isJap;

      public:
        InputProcessorImpl(const TF_INPUTPROCESSORPROFILE profile) : profile(profile)
        {
            type   = profile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT ?
                         IngameIME::InputProcessorType::KeyboardLayout :
                         IngameIME::InputProcessorType::TextService;
            name   = getInputProcessorName(profile);
            locale = IngameIME::InternalLocale::getLocale(profile.langid);
            isJap  = locale->locale.compare(0, 2, L"ja") == 0;
        }

        ~InputProcessorImpl()
        {
            weakRefs.erase(profile);
        }

      public:
        static std::shared_ptr<const InputProcessorImpl> getInputProcessor(const TF_INPUTPROCESSORPROFILE profile)
        {
            auto iter = weakRefs.find(profile);

            std::shared_ptr<const InputProcessorImpl> proc;

            // Create new proc if not exist or expired
            if (iter == weakRefs.end() || !(proc = (*iter).second.lock())) {
                proc              = std::make_shared<InputProcessorImpl>(profile);
                weakRefs[profile] = proc;
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
    std::map<TF_INPUTPROCESSORPROFILE, std::weak_ptr<const InputProcessorImpl>, InputProcessorImpl::CompareProfile>
        InputProcessorImpl::weakRefs = {};

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
                if (var.intVal & TF_CONVERSIONMODE_NATIVE) {
                    modes.push_back(L"Native");

                    if (activeProc->isJap)
                        if (var.intVal & TF_CONVERSIONMODE_KATAKANA)
                            modes.push_back(L"Katakana");
                        else
                            modes.push_back(L"Hiragana");
                }
                else
                    modes.push_back(L"AlphaNumeric");

                if (var.intVal & TF_CONVERSIONMODE_FULLSHAPE)
                    modes.push_back(L"FullShape");
                else
                    modes.push_back(L"HalfShape");
            }

            return modes;
        }

      public:
        IngameIME::InputProcessorContext getCtx()
        {
            IngameIME::InputProcessorContext result;
            result.proc  = activeProc;
            result.modes = getInputModes();

            return result;
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

            IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::FullUpdate, getCtx());

            COM_HR_END();
            COM_HR_RET();
        }

        HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override
        {
            COM_HR_BEGIN(S_OK);

            if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
                IngameIME::Global::getInstance().runCallback(IngameIME::InputProcessorState::InputModeUpdate, getCtx());

            COM_HR_END();
            COM_HR_RET();
        }
    };
}// namespace libtf

namespace libimm {
    class InputProcessorImpl : public IngameIME::InputProcessor {
      protected:
        static std::map<HKL, std::weak_ptr<const InputProcessorImpl>> weakRefs;

      protected:
        const HKL hkl;

      public:
        /**
         * @brief If this is a Japanese InputProcessor
         *
         */
        bool isJap;

      protected:
        static std::wstring getInputProcessorName(HKL hkl)
        {
            if (ImmIsIME(hkl)) {
                auto size = ImmGetDescriptionW(hkl, NULL, 0);

                auto buf = std::make_unique<WCHAR[]>(size);
                ImmGetDescriptionW(hkl, buf.get(), size);

                return std::wstring(buf.get(), size);
            }
            else
                return IngameIME::getKeyboardLayoutName(MAKELCID(LOWORD(hkl), SORT_DEFAULT));
        }

      public:
        InputProcessorImpl(const HKL hkl) : hkl(hkl)
        {
            type   = ImmIsIME(hkl) ? IngameIME::InputProcessorType::TextService :
                                     IngameIME::InputProcessorType::KeyboardLayout;
            locale = IngameIME::InternalLocale::getLocale(MAKELCID(LOWORD(hkl), SORT_DEFAULT));
            name   = getInputProcessorName(hkl);
            isJap  = locale->locale.compare(0, 2, L"ja") == 0;
        }

        ~InputProcessorImpl() noexcept
        {
            weakRefs.erase(hkl);
        }

      public:
        static std::shared_ptr<const InputProcessorImpl> getInputProcessor(const HKL hkl)
        {
            auto iter = weakRefs.find(hkl);

            std::shared_ptr<const InputProcessorImpl> proc;

            // Create new proc if not exist or expired
            if (iter == weakRefs.end() || !(proc = (*iter).second.lock())) {
                proc          = std::make_shared<InputProcessorImpl>(hkl);
                weakRefs[hkl] = proc;
            }

            return proc;
        }

        static std::shared_ptr<const InputProcessorImpl> getActiveInputProcessor()
        {
            // Pass Null to get current thread
            auto hkl = GetKeyboardLayout(NULL);
            return InputProcessorImpl::getInputProcessor(hkl);
        }

      public:
        /**
         * @brief Set active InputProcessor for current InputProcessor
         *
         */
        virtual void setActivated() const noexcept override
        {
            ActivateKeyboardLayout(hkl, 0);
        }
    };
    std::map<HKL, std::weak_ptr<const InputProcessorImpl>> InputProcessorImpl::weakRefs = {};
}// namespace libimm