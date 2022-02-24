#pragma once
#include <algorithm>
#include <list>
#include <map>

#include <Windows.h>
#pragma comment(lib, "imm32.lib")

#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <msctf.h>

#include "IngameIME.hpp"
#include "InputProcessor.hpp"

#include "ComBSTR.hpp"
#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "FormatUtil.hpp"
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
        static std::wstring getKeyboardLayoutName(LANGID langId)
        {
            auto result = format(L"[KL: 0x%1!08x!]", langId);

            HKEY layouts;
            if (ERROR_SUCCESS == RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                                               L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
                                               0,
                                               KEY_READ,
                                               &layouts)) {
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

        static std::wstring getTextServiceName(const TF_INPUTPROCESSORPROFILE profile)
        {
            auto result = format(L"[TIP: {%1!08x!-0x%1!04x!-0x%1!04x!-0x%1!04x!-0x%1!012x!}]",
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

        static std::wstring getImmName(HKL hkl)
        {
            auto size = ImmGetDescriptionW(hkl, NULL, 0) + 1;

            if (size == 0) return format(L"[IMM: %1!08x!]", hkl);

            auto buf = std::make_unique<WCHAR[]>(size);
            ImmGetDescriptionW(hkl, buf.get(), size);

            return std::wstring(buf.get(), size);
        }

        /**
         * @brief Check if the HKL correspond to a Imm InputMethod
         *
         * @param hkl hkl to check
         * @return true - Imm InputMethod
         * @return false - Normal KeyboardLayout
         */
        static bool isImm(HKL hkl)
        {
            return (0xF000 & HIWORD(hkl)) == 0xE000;
        }

        static std::wstring getInputProcessorName(const TF_INPUTPROCESSORPROFILE profile)
        {
            switch (profile.dwProfileType) {
                case TF_PROFILETYPE_INPUTPROCESSOR: return getTextServiceName(profile);
                case TF_PROFILETYPE_KEYBOARDLAYOUT:
                    if (isImm(profile.hkl))
                        return getImmName(profile.hkl);
                    else
                        return getKeyboardLayoutName(profile.langid);
                default: return L"[InputProcessor: unknown]";
            }
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
            type   = profile.dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR || isImm(profile.hkl) ?
                         IngameIME::InputProcessorType::TextService :
                         IngameIME::InputProcessorType::KeyboardLayout;
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

        static std::shared_ptr<const InputProcessorImpl> getActiveInputProcessor()
        {
            COM_HR_BEGIN(S_OK);

            if (!IsGUIThread(false)) THR_HR(UI_E_WRONG_THREAD);

            libtf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(libtf::createInputProcessorProfiles(&inputProcessorProfiles));
            libtf::ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
                                                                           inputProcessorProfiles);

            TF_INPUTPROCESSORPROFILE profile;
            CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile));
            return InputProcessorImpl::getInputProcessor(profile);

            COM_HR_END();
            COM_HR_THR();

            // Should not reach here
            return nullptr;
        }

        static std::list<std::shared_ptr<const IngameIME::InputProcessor>> getInputProcessors()
        {
            std::list<std::shared_ptr<const IngameIME::InputProcessor>> result;

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

                // Reach end
                if (fetch == 0) break;

                // InputProcessor not enabled can't be activated
                if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED)) continue;

                result.push_back(InputProcessorImpl::getInputProcessor(profile[0]));
            }

            COM_HR_END();

            return result;
        }

      public:
        virtual void setActivated() const override
        {
            COM_HR_BEGIN(S_OK);

            if (!IsGUIThread(false)) THR_HR(UI_E_WRONG_THREAD);

            libtf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(libtf::createInputProcessorProfiles(&inputProcessorProfiles));
            libtf::ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
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
}// namespace IngameIME

namespace libtf {
    class InputProcessorHandler : public ComObjectBase,
                                  public ITfInputProcessorProfileActivationSink,
                                  public ITfCompartmentEventSink {
      public:
        ComQIPtr<ITfCompartmentMgr> compMgr{IID_ITfCompartmentMgr};
        ComPtr<ITfCompartment>      mode;

      public:
        InputProcessorHandler()
        {
            COM_HR_BEGIN(S_OK);

            ComPtr<ITfThreadMgr> threadMgr;
            CHECK_HR(getThreadMgr(&threadMgr));
            compMgr = threadMgr;

            CHECK_HR(compMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &mode));

            COM_HR_END();
            COM_HR_THR();
        }

      protected:
        IngameIME::InputProcessorContext getCtx()
        {
            IngameIME::InputProcessorContext result;

            auto activeProc = IngameIME::InputProcessorImpl::getActiveInputProcessor();
            result.proc     = activeProc;

            VARIANT var;
            mode->GetValue(&var);

            if (activeProc->type == IngameIME::InputProcessorType::KeyboardLayout)
                result.modes.push_back(L"AlphaNumeric");
            else {
                if (var.intVal & TF_CONVERSIONMODE_NATIVE) {
                    result.modes.push_back(L"Native");

                    if (activeProc->isJap)
                        if (var.intVal & TF_CONVERSIONMODE_KATAKANA)
                            result.modes.push_back(L"Katakana");
                        else
                            result.modes.push_back(L"Hiragana");
                }
                else
                    result.modes.push_back(L"AlphaNumeric");

                if (var.intVal & TF_CONVERSIONMODE_FULLSHAPE)
                    result.modes.push_back(L"FullShape");
                else
                    result.modes.push_back(L"HalfShape");
            }

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