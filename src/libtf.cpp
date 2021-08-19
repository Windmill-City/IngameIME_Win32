#include "libtf.h"
#include "InputContext.hpp"
#include "TfFunction.hpp"

#include <string>

#include <stdio.h>
#include <winreg.h>

using namespace libtf;
struct libtf_tagInputContext
{
    CComPtr<libtf::CInputContext> ctx;
};
typedef struct libtf_tagInputContext InputContext_t;

#pragma region InputProcesser Profile
/**
 * @brief Copy data from tfProfile to libtf Profile
 */
void copyProfileData(TF_INPUTPROCESSORPROFILE tfProfile, libtf_InputProcessorProfile_t& profile)
{
    profile.profileType = tfProfile.dwProfileType;
    profile.langId      = tfProfile.langid;
    profile.hkl         = tfProfile.hkl;
    profile.activated   = tfProfile.dwFlags & TF_IPP_FLAG_ACTIVE;
    memcpy(&profile.clsid, &tfProfile.clsid, sizeof(GUID));
    memcpy(&profile.catid, &tfProfile.catid, sizeof(GUID));
    memcpy(&profile.guidProfile, &tfProfile.guidProfile, sizeof(GUID));
}

/**
 * @brief Get available input processor profies for the calling thread
 *
 * @param profiles Pointer to an array of libtf_InputProcessorProfile_t.
 * This array must be at least maxSize elements in size
 * @param maxSize the max size of the profiles array
 * @param fetched if profiles is NULL, return the max size of the profiles, otherwise, return the fetched size.
 * The fetched size can change from one call to the next, pay attention to it.
 */
HRESULT libtf_get_input_processors(libtf_InputProcessorProfile_t* profiles, uint32_t maxSize, uint32_t* fetched)
{
    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
    CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

    CComPtr<IEnumTfInputProcessorProfiles> enumProfiles;
    // Pass 0 to langid to enum all profiles
    CHECK_HR(inputProcessorMgr->EnumProfiles(0, &enumProfiles));

    uint32_t                 number = 0;
    TF_INPUTPROCESSORPROFILE profile[1];
    while (true) {
        ULONG fetch;
        CHECK_HR(enumProfiles->Next(1, profile, &fetch));

        // No more
        if (fetch == 0) break;

        // InputProcessor not enabled can't be use
        if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED)) continue;

        // Copy data
        if (profiles) {
            // Reach max size
            if (number >= maxSize) break;
            copyProfileData(profile[0], profiles[number]);
        }

        number++;
    }
    *fetched = number;

    return S_OK;
}

/**
 * @brief Get active input processor profie for the calling thread
 */
HRESULT libtf_get_active_input_processor(libtf_InputProcessorProfile_t* profile)
{
    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
    CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

    TF_INPUTPROCESSORPROFILE tfProfile;
    CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, &tfProfile));

    copyProfileData(tfProfile, *profile);

    return S_OK;
}

/**
 * @brief Set active input processor profie for the calling thread
 */
HRESULT libtf_set_active_input_processor(libtf_InputProcessorProfile_t profile)
{
    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));
    CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

    CHECK_HR(inputProcessorMgr->ActivateProfile(profile.profileType,
                                                profile.langId,
                                                profile.clsid,
                                                profile.guidProfile,
                                                profile.hkl,
                                                TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE));

    return S_OK;
}

/**
 * @brief Get the locale of the input processor
 *
 * @param BSTR* Pointer to a BSTR value that receives the locale string. The caller is responsible for freeing
 * this memory using SysFreeString when it is no longer required.
 */
HRESULT libtf_get_input_processor_locale(libtf_InputProcessorProfile_t profile, BSTR* locale)
{
    LCID    lcid = MAKELCID(profile.langId, SORT_DEFAULT);
    wchar_t buf[85];
    GetLocaleInfoW(lcid, LOCALE_SNAME, buf, 85);
    *locale = SysAllocString(buf);
    return S_OK;
}

/**
 * @brief Get the localized name of the locale
 *
 * @param BSTR locale
 * @param BSTR* Pointer to a BSTR value that receives the name string. The caller is responsible for freeing
 * this memory using SysFreeString when it is no longer required.
 */
HRESULT libtf_get_locale_name(BSTR locale, BSTR* name)
{
    wchar_t buf[128];
    GetLocaleInfoEx(locale, LOCALE_SLOCALIZEDDISPLAYNAME, buf, 128);
    *name = SysAllocString(buf);
    return S_OK;
}

/**
 * @brief Get the localized name of the input processor
 *
 * @param BSTR Pointer to a BSTR value that receives the description string. The caller is responsible for freeing this
 * memory using SysFreeString when it is no longer required.
 */
HRESULT libtf_get_input_processor_desc(libtf_InputProcessorProfile_t profile, BSTR* desc)
{
    switch (profile.profileType) {
        case TF_PROFILETYPE_INPUTPROCESSOR: {
            CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
            CHECK_HR(createInputProcessorProfiles(&inputProcessorProfiles));

            CHECK_HR(inputProcessorProfiles->GetLanguageProfileDescription(
                profile.clsid, profile.langId, profile.guidProfile, desc));
        } break;
        case TF_PROFILETYPE_KEYBOARDLAYOUT: {
            HKEY layouts;
            CHECK_ES(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                                  TEXT("SYSTEM\\CurrentControlSet\\Control\\Keyboard Layouts"),
                                  0,
                                  KEY_READ,
                                  &layouts));

            HRESULT hr;
            // the key of the keyboard layout is its langid
            char layoutKey[9];
            snprintf(layoutKey, 9, "%08x", profile.langId);
            HKEY layout;
            if (NOT_ES(hr = RegOpenKeyExA(layouts, layoutKey, 0, KEY_READ, &layout))) goto CloseParentKey;

            DWORD size;
            // Get data size first
            if (NOT_ES(
                    hr = RegGetValueW(
                        layout, NULL, L"Layout Display Name", RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND, NULL, NULL, &size)))
                goto CloseSubKey;

            {
                std::unique_ptr<uint8_t[]> buf(new uint8_t[size]);
                // Get resource key of the name
                if (NOT_ES(hr = RegGetValueW(layout,
                                             NULL,
                                             L"Layout Display Name",
                                             RRF_RT_REG_EXPAND_SZ | RRF_NOEXPAND,
                                             NULL,
                                             buf.get(),
                                             &size)))
                    goto CloseSubKey;

                // Get the layout name by resource key
                std::unique_ptr<wchar_t[]> layoutName(new wchar_t[KL_NAMELENGTH]);
                if (FAILED(hr = SHLoadIndirectString((wchar_t*)buf.get(), layoutName.get(), KL_NAMELENGTH, NULL)))
                    goto CloseSubKey;

                // Return result
                *desc = SysAllocString(layoutName.get());
                CHECK_OOM(*desc);
            }
        CloseSubKey:
            RegCloseKey(layout);
        CloseParentKey:
            RegCloseKey(layouts);
            return hr;
        } break;
        default: return E_INVALIDARG;
    }
    return S_OK;
}
#pragma endregion

#pragma region Context
/**
 * @brief Create input context for the calling thread
 */
HRESULT libtf_create_ctx(libtf_pInputContext* ctx)
{
    auto context = new InputContext_t();
    *ctx         = context;
    context->ctx = new CInputContext();

    CHECK_HR(context->ctx->initialize());

    return S_OK;
}

/**
 * @brief Dispose the input context, the pointer to the context will be invailed
 */
HRESULT libtf_dispose_ctx(libtf_pInputContext ctx)
{
    HRESULT hr = ctx->ctx->dispose();
    delete ctx;
    return hr;
}

/**
 * @brief Terminate active composition of the context
 */
HRESULT libtf_terminate_composition(libtf_pInputContext ctx)
{
    return ctx->ctx->terminateComposition();
}

/**
 * @brief Set current selected Candidate for the Candidate List
 *
 * @param index index of the Candidate
 */
LIBTF_EXPORT HRESULT libtf_set_candidate_list_sel(libtf_pInputContext ctx, uint32_t index)
{
    return ctx->ctx->m_candHandler->setSelection(index);
}

/**
 * @brief Finalize by specific Candidate for the Candidate List
 *
 * @param index index of the Candidate
 */
LIBTF_EXPORT HRESULT libtf_final_candidate_list_sel(libtf_pInputContext ctx, uint32_t index)
{
    return ctx->ctx->m_candHandler->setFinalize(index);
}

/**
 * @brief Set input method state of the context
 *
 * @param bool true to enable the input method, false to disable it
 */
HRESULT libtf_set_im_state(libtf_pInputContext ctx, bool enable)
{
    return ctx->ctx->setIMState(enable);
}

/**
 * @brief Get input method(IM) state of the context
 *
 * @param bool returns true if IM is enabled, false otherwise
 */
HRESULT libtf_get_im_state(libtf_pInputContext ctx, bool* imState)
{
    return ctx->ctx->getIMState(imState);
}

/**
 * @brief This method should be called from the WndProc of the ui-thread,
 * of whom should be the creator of this context
 *
 * @param hWnd The window who receives the message
 * @param message can be one of WM_SETFOCUS/WM_KILLFOCUS
 */
HRESULT libtf_on_focus_msg(libtf_pInputContext ctx, HWND hWnd, UINT message)
{
    return ctx->ctx->onFocusMsg(hWnd, message);
}

/**
 * @brief Get current focused window of the context
 *
 * @param HWND* current focused window, this can be NULL if no window get focused
 */
HRESULT libtf_get_focus_wnd(libtf_pInputContext ctx, HWND* hWnd)
{
    return ctx->ctx->getFocusedWnd(hWnd);
}

/**
 * @brief Set Conversion Mode of the context
 */
HRESULT libtf_set_conversion_mode(libtf_pInputContext ctx, libtf_ConversionMode mode)
{
    return ctx->ctx->m_conversionHander->setConversionMode(mode);
}

/**
 * @brief Set Sentence Mode of the context
 */
HRESULT libtf_set_sentence_mode(libtf_pInputContext ctx, libtf_SentenceMode mode)
{
    return ctx->ctx->m_sentenceHander->setSentenceMode(mode);
}

/**
 * @brief Get Conversion Mode of the context
 */
HRESULT libtf_get_conversion_mode(libtf_pInputContext ctx, libtf_ConversionMode* mode)
{
    return ctx->ctx->m_conversionHander->getConversionMode(mode);
}

/**
 * @brief Get Sentence Mode of the context
 */
HRESULT libtf_get_sentence_mode(libtf_pInputContext ctx, libtf_SentenceMode* mode)
{
    return ctx->ctx->m_sentenceHander->getSentenceMode(mode);
}

/**
 * @brief Set Full Screen mode of the context
 */
HRESULT libtf_set_full_screen(libtf_pInputContext ctx, bool isFullScreen)
{
    ctx->ctx->m_fullScHandler->m_isFullScreen = isFullScreen;
    return S_OK;
}

/**
 * @brief Set if input method should show its Candidate Window of the context
 */
HRESULT libtf_set_show_candidate_list_wnd(libtf_pInputContext ctx, bool show)
{
    ctx->ctx->m_candHandler->m_show = show;
    return S_OK;
}

/**
 * @brief Get Full Screen mode of the context
 */
HRESULT libtf_get_full_screen(libtf_pInputContext ctx, bool* isFullScreen)
{
    *isFullScreen = ctx->ctx->m_fullScHandler->m_isFullScreen;
    return S_OK;
}

/**
 * @brief Get if input method should show its Candidate Window of the context
 */
HRESULT libtf_get_show_candidate_list_wnd(libtf_pInputContext ctx, bool* show)
{
    *show = ctx->ctx->m_candHandler->m_show;
    return S_OK;
}
#pragma endregion

#pragma region setCallback
/**
 * @brief Set Composition Callback of the context
 */
HRESULT libtf_set_composition_callback(libtf_pInputContext ctx, libtf_CallbackComposition callback, void* userData)
{
    ctx->ctx->m_compositionHandler->m_sigComposition = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}

/**
 * @brief Set Commit Callback of the context
 */
HRESULT libtf_set_commit_callback(libtf_pInputContext ctx, libtf_CallbackCommit callback, void* userData)
{
    ctx->ctx->m_compositionHandler->m_commitHandler->m_sigCommit = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}

/**
 * @brief Set PreEdit Bounding Box Callback of the context
 */
HRESULT libtf_set_bounding_box_callback(libtf_pInputContext ctx, libtf_CallbackBoundingBox callback, void* userData)
{
    ctx->ctx->m_compositionHandler->m_sigBoundingBox = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}

/**
 * @brief Set Candidate List Callback of the context
 */
HRESULT libtf_set_candidate_list_callback(libtf_pInputContext ctx, libtf_CallbackCandidateList callback, void* userData)
{
    ctx->ctx->m_candHandler->m_sigCandidateList = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}

/**
 * @brief Set Conversion mode Callback of the context
 */
HRESULT
libtf_set_conversion_mode_callback(libtf_pInputContext ctx, libtf_CallbackConversionMode callback, void* userData)
{
    ctx->ctx->m_conversionHander->sigConversionMode = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}

/**
 * @brief Set Sentence mode Callback of the context
 */
HRESULT libtf_set_sentence_mode_callback(libtf_pInputContext ctx, libtf_CallbackSentenceMode callback, void* userData)
{
    ctx->ctx->m_sentenceHander->sigSentenceMode = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}

/**
 * @brief Set Input Processor Callback of the context
 */
HRESULT
libtf_set_input_processor_callback(libtf_pInputContext ctx, libtf_CallbackInputProcessor callback, void* userData)
{
    ctx->ctx->m_inputProcessor->sigInputProcessor = std::bind(callback, std::placeholders::_1, userData);
    return S_OK;
}
#pragma endregion