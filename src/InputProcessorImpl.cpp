#include "common/InputProcessorImpl.hpp"

namespace IngameIME
{
InternalLocale::Singleton::RefHolderType     InternalLocale::WeakRefs           = {};
InputProcessorImpl::Singleton::RefHolderType InputProcessorImpl::WeakRefs       = {};
std::mutex                                   InputProcessorImpl::RefHolderMutex = std::mutex();
std::mutex                                   InternalLocale::RefHolderMutex     = std::mutex();

std::wstring InternalLocale::getLocaleString(const LANGID langid)
{
    LCID lcid = MAKELCID(langid, SORT_DEFAULT);

    int  size = GetLocaleInfoW(lcid, LOCALE_SNAME, NULL, 0);
    auto buf  = std::make_unique<wchar_t[]>(size);

    GetLocaleInfoW(lcid, LOCALE_SNAME, buf.get(), size);

    return std::wstring(buf.get(), size);
}

std::wstring InternalLocale::getLocaleName(const LANGID langid)
{
    LCID lcid = MAKELCID(langid, SORT_DEFAULT);

    int  size = GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, NULL, 0);
    auto buf  = std::make_unique<wchar_t[]>(size);

    GetLocaleInfoW(lcid, LOCALE_SLOCALIZEDDISPLAYNAME, buf.get(), size);

    return std::wstring(buf.get(), size);
}

InternalLocale::InternalLocale(const LANGID langId) noexcept
    : Singleton(langId)
    , langId(langId)
{
    locale = getLocaleString(langId);
    name   = getLocaleName(langId);
}

std::wstring InputProcessorImpl::getKeyboardLayoutName(LANGID langId)
{
    auto result = format(L"[KL: 0x%1!08x!]", langId);

    HKEY layouts;
    if (ERROR_SUCCESS
        == RegOpenKeyExW(HKEY_LOCAL_MACHINE,
                         L"SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts",
                         0,
                         KEY_READ,
                         &layouts))
    {
        // the key of the keyboard layout is its langid
        char layoutKey[9];
        snprintf(layoutKey, 9, "%08x", langId);

        HKEY layout;
        if (ERROR_SUCCESS == RegOpenKeyExA(layouts, layoutKey, 0, KEY_READ, &layout))
        {
            // Get data size first
            DWORD size;
            if (ERROR_SUCCESS
                == RegGetValueW(layout,
                                NULL,
                                L"Layout Display Name",
                                RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                NULL,
                                NULL,
                                &size))
            {
                // Get resource key of the name
                auto resKey = std::make_unique<wchar_t[]>(size);
                if (ERROR_SUCCESS
                    == RegGetValueW(layout,
                                    NULL,
                                    L"Layout Display Name",
                                    RRF_RT_REG_SZ | RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                    NULL,
                                    resKey.get(),
                                    &size))
                {
                    // Get the layout name by resource key
                    wchar_t layoutName[64];
                    HRESULT hr;
                    if (SUCCEEDED(hr = SHLoadIndirectString(resKey.get(), layoutName, 64, NULL)))
                    {
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

std::wstring InputProcessorImpl::getTextServiceName(const TF_INPUTPROCESSORPROFILE& profile)
{
    auto result = format(L"[TIP: {%1!08x!-0x%1!04x!-0x%1!04x!-0x%1!04x!-0x%1!012x!}]",
                         profile.clsid.Data1,
                         profile.clsid.Data2,
                         profile.clsid.Data3,
                         profile.clsid.Data4);

    COM_HR_BEGIN(S_OK);

    tf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(tf::createInputProcessorProfiles(&inputProcessorProfiles));

    tf::ComBSTR name;
    CHECK_HR(inputProcessorProfiles->GetLanguageProfileDescription(profile.clsid,
                                                                   profile.langid,
                                                                   profile.guidProfile,
                                                                   &name));

    result = std::wstring(name.bstr);

    COM_HR_END();

    return result;
}

std::wstring InputProcessorImpl::getImmName(HKL hkl)
{
    auto size = ImmGetDescriptionW(hkl, NULL, 0) + 1;

    if (size == 0) return format(L"[IMM: %1!08x!]", hkl);

    auto buf = std::make_unique<WCHAR[]>(size);
    ImmGetDescriptionW(hkl, buf.get(), size);

    return std::wstring(buf.get(), size);
}

bool InputProcessorImpl::isImm(HKL hkl)
{
    return (0xF000 & HIWORD(hkl)) == 0xE000;
}

std::wstring InputProcessorImpl::getInputProcessorName(const TF_INPUTPROCESSORPROFILE& profile)
{
    switch (profile.dwProfileType)
    {
    case TF_PROFILETYPE_INPUTPROCESSOR:
        return getTextServiceName(profile);
    case TF_PROFILETYPE_KEYBOARDLAYOUT:
        if (isImm(profile.hkl))
            return getImmName(profile.hkl);
        else
            return getKeyboardLayoutName(profile.langid);
    default:
        return L"[InputProcessor: unknown]";
    }
}

InputProcessorImpl::InputProcessorImpl(const TF_INPUTPROCESSORPROFILE& profile)
    : Singleton(profile)
    , profile(profile)
{
    type   = profile.dwProfileType == TF_PROFILETYPE_INPUTPROCESSOR || isImm(profile.hkl)
               ? InputProcessorType::TextService
               : InputProcessorType::KeyboardLayout;
    name   = getInputProcessorName(profile);
    locale = InternalLocale::getOrCreate(profile.langid);
    isJap  = locale->locale.compare(0, 2, L"ja") == 0;
}

std::shared_ptr<const InputProcessorImpl> InputProcessorImpl::getActiveInputProcessor()
{
    COM_HR_BEGIN(S_OK);

    if (!IsGUIThread(false)) THR_HR(UI_E_WRONG_THREAD);

    tf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(tf::createInputProcessorProfiles(&inputProcessorProfiles));
    tf::ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
                                                                inputProcessorProfiles);

    TF_INPUTPROCESSORPROFILE profile;
    CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &profile));
    return InputProcessorImpl::getOrCreate(profile);

    COM_HR_END();
    COM_HR_THR();

    // Should not reach here
    return nullptr;
}

std::list<std::shared_ptr<const InputProcessor>> InputProcessorImpl::getInputProcessors()
{
    std::list<std::shared_ptr<const InputProcessor>> result;

    COM_HR_BEGIN(S_OK);

    if (!IsGUIThread(false)) break;

    tf::ComPtr<ITfInputProcessorProfiles> profiles;
    CHECK_HR(tf::createInputProcessorProfiles(&profiles));
    tf::ComQIPtr<ITfInputProcessorProfileMgr> procMgr(IID_ITfInputProcessorProfileMgr, profiles);

    tf::ComPtr<IEnumTfInputProcessorProfiles> enumProfiles;
    // Pass 0 to langid to enum all profiles
    CHECK_HR(procMgr->EnumProfiles(0, &enumProfiles));

    TF_INPUTPROCESSORPROFILE profile[1];
    while (true)
    {
        ULONG fetch;
        CHECK_HR(enumProfiles->Next(1, profile, &fetch));

        // Reach end
        if (fetch == 0) break;

        // InputProcessor not enabled can't be activated
        if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED)) continue;

        result.push_back(InputProcessorImpl::getOrCreate(profile[0]));
    }

    COM_HR_END();

    return result;
}

void InputProcessorImpl::setActivated() const
{
    COM_HR_BEGIN(S_OK);

    if (!IsGUIThread(false)) THR_HR(UI_E_WRONG_THREAD);

    tf::ComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(tf::createInputProcessorProfiles(&inputProcessorProfiles));
    tf::ComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr(IID_ITfInputProcessorProfileMgr,
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

bool ComparableProfile::operator<(const ComparableProfile& s2) const
{
    if (this->dwProfileType == s2.dwProfileType)
        if (this->dwProfileType == TF_PROFILETYPE_KEYBOARDLAYOUT)
            return this->hkl < s2.hkl;
        else
            return memcmp(&this->clsid, &s2.clsid, sizeof(CLSID)) < 0;
    else
        return this->dwProfileType < s2.dwProfileType;
}

ComparableProfile::ComparableProfile(const TF_INPUTPROCESSORPROFILE& profile)
{
    memcpy(this, &profile, sizeof(profile));
}
} // namespace IngameIME
