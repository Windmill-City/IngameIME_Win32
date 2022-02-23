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
#include "FormatUtil.hpp"
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
        auto result = format(L"[KL:0x%1!08x!]", langId);

        HKEY layouts;
        if (ERROR_SUCCESS ==
            RegOpenKeyExW(
                HKEY_LOCAL_MACHINE, L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts", 0, KEY_READ, &layouts)) {
            // the key of the keyboard layout is its langid
            char layoutKey[9];
            snprintf(layoutKey, 9, "%08x", langId);

            HKEY layout;
            if (ERROR_SUCCESS == RegOpenKeyExA(layouts, layoutKey, 0, KEY_READ, &layout)) {
                // Get data size first
                DWORD size;
                if (ERROR_SUCCESS == RegGetValueW(layout,
                                                  NULL,
                                                  L"Layout Display Name",
                                                  RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                                  NULL,
                                                  NULL,
                                                  &size)) {
                    // Get resource key of the name
                    auto resKey = std::make_unique<wchar_t[]>(size);
                    if (ERROR_SUCCESS == RegGetValueW(layout,
                                                      NULL,
                                                      L"Layout Display Name",
                                                      RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                                      NULL,
                                                      resKey.get(),
                                                      &size)) {
                        // Get the layout name by resource key
                        wchar_t layoutName[64];
                        HRESULT hr;
                        if (SUCCEEDED(hr = SHLoadIndirectString(resKey.get(), layoutName, 64, NULL))) {
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
    std::wstring getTextServiceName(const TF_INPUTPROCESSORPROFILE profile)
    {
        auto result = format(L"[TIP:{%1!08x!-0x%1!04x!-0x%1!04x!-0x%1!04x!-0x%1!012x!}]",
                             profile.clsid.Data1,
                             profile.clsid.Data2,
                             profile.clsid.Data3,
                             profile.clsid.Data4);

        COM_HR_BEGIN(S_OK);

        libtf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
        CHECK_HR(libtf::createInputProcessorProfiles(&inputProcessorProfiles));

        libtf::ComBSTR name;
        CHECK_HR(inputProcessorProfiles->GetLanguageProfileDescription(
            profile.clsid, profile.langid, profile.guidProfile, &name));

        result = std::wstring(name.bstr);

        COM_HR_END();

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
            switch (profile.dwProfileType) {
                case TF_PROFILETYPE_INPUTPROCESSOR: return IngameIME::getTextServiceName(profile);
                case TF_PROFILETYPE_KEYBOARDLAYOUT: return IngameIME::getKeyboardLayoutName(profile.langid);
            }
            return L"[unknown]";
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
        static bool isIMM(HKL hkl)
        {
            return (0xF000 & HIWORD(hkl)) == 0xE000;
        }

        static std::wstring getInputProcessorName(HKL hkl)
        {
            auto size = ImmGetDescriptionW(hkl, NULL, 0) + 1;

            if (size == 0) return L"[IMM:unknown]";

            auto buf = std::make_unique<WCHAR[]>(size);
            ImmGetDescriptionW(hkl, buf.get(), size);

            return std::wstring(buf.get(), size);
        }

      public:
        InputProcessorImpl(const HKL hkl) : hkl(hkl)
        {
            if (!isIMM(hkl)) throw std::exception("Only support IMM!");
            type   = IngameIME::InputProcessorType::TextService;
            locale = IngameIME::InternalLocale::getLocale(MAKELCID(LOWORD(hkl), SORT_DEFAULT));
            name   = getInputProcessorName(hkl);
            isJap  = locale->locale.compare(0, 2, L"ja") == 0;
        }

        InputProcessorImpl(const TF_INPUTPROCESSORPROFILE profile) : hkl(profile.hkl)
        {
            if (profile.dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT) {
                type = IngameIME::InputProcessorType::KeyboardLayout;
                name = IngameIME::getKeyboardLayoutName(profile.langid);
            }
            else {
                type = IngameIME::InputProcessorType::TextService;
                name = IngameIME::getTextServiceName(profile);
            }
            locale = IngameIME::InternalLocale::getLocale(profile.langid);
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

        static std::shared_ptr<const InputProcessorImpl> getInputProcessor(const TF_INPUTPROCESSORPROFILE profile)
        {
            auto iter = weakRefs.find(profile.hkl);

            std::shared_ptr<const InputProcessorImpl> proc;

            // Create new proc if not exist or expired
            if (iter == weakRefs.end() || !(proc = (*iter).second.lock())) {
                proc                  = std::make_shared<InputProcessorImpl>(profile);
                weakRefs[profile.hkl] = proc;
            }

            return proc;
        }

        static std::shared_ptr<const InputProcessorImpl> getActiveInputProcessor()
        {
            // Pass Null to get current thread
            auto hkl = GetKeyboardLayout(NULL);
            if (isIMM(hkl))
                return InputProcessorImpl::getInputProcessor(hkl);
            else {
                COM_HR_BEGIN(S_OK);

                libtf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
                CHECK_HR(libtf::createInputProcessorProfiles(&inputProcessorProfiles));
                libtf::ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
                                                                               inputProcessorProfiles);

                TF_INPUTPROCESSORPROFILE profile;
                CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile));
                return InputProcessorImpl::getInputProcessor(profile);

                COM_HR_END();
                COM_HR_THR();
            }
            // Should not reach here
            return nullptr;
        }

        static std::list<std::shared_ptr<const IngameIME::InputProcessor>> getInputProcessors()
        {
            int size = GetKeyboardLayoutList(0, NULL);

            auto buf = std::make_unique<HKL[]>(size);
            GetKeyboardLayoutList(size, buf.get());

            std::list<std::shared_ptr<const IngameIME::InputProcessor>> result;

            for (size_t i = 0; i < size; i++) {
                auto hkl = buf[i];
                if (isIMM(hkl)) result.push_back(InputProcessorImpl::getInputProcessor(hkl));
            }

            COM_HR_BEGIN(S_OK);

            if (!IsGUIThread(false)) break;

            libtf::ComPtr<ITfInputProcessorProfiles> profiles;
            CHECK_HR(libtf::createInputProcessorProfiles(&profiles));
            libtf::ComQIPtr<ITfInputProcessorProfileMgr> procMgr(IID_ITfInputProcessorProfileMgr, profiles);

            libtf::ComPtr<IEnumTfInputProcessorProfiles> enumProfiles;
            // Pass 0 to langid to enum all profiles
            CHECK_HR(procMgr->EnumProfiles(0, &enumProfiles));

            TF_INPUTPROCESSORPROFILE profile[1];
            while (true) {
                ULONG fetch;
                CHECK_HR(enumProfiles->Next(1, profile, &fetch));

                // No more
                if (fetch == 0) break;

                // InputProcessor not enabled can't be activated
                if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED) || isIMM(profile->hkl)) continue;

                result.push_back(InputProcessorImpl::getInputProcessor(profile[0]));
            }
            COM_HR_END();

            return result;
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