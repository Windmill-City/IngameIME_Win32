#include "libtf.h"
#include <string>

using namespace libtf;
#pragma region InputProcesser Profile
/**
 * @brief Get available input processor profies of current thread
 * 
 * @param profiles Pointer to an array of libtf_InputProcessorProfile_t.
 *                  This array must be at least maxSize elements in size
 * @param maxSize the max size of the profiles array
 * @param fetched if profiles is NULL, return the max number can obtain,
 *                otherwise, return the number of elements actually obtained
 */
HRESULT libtf_get_input_processors(libtf_InputProcessorProfile_t *profiles, size_t maxSize, size_t *fetched)
{
    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(inputProcessorProfiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER));
    CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

    CComPtr<IEnumTfInputProcessorProfiles> enumProfiles;
    //Pass 0 to langid to enum all profiles
    CHECK_HR(inputProcessorMgr->EnumProfiles(0, &enumProfiles));

    size_t number = 0;
    libtf_InputProcessorProfile_t profile[1];
    while (true)
    {
        ULONG fetch;
        CHECK_HR(enumProfiles->Next(1, profile, &fetch));

        //No more
        if (fetch == 0) break;

        //InputProcessor not enabled can't be use
        if (!(profile[0].dwFlags & TF_IPP_FLAG_ENABLED)) continue;

        //Copy data
        if (profiles)
        {
            //Reach max size
            if (number >= maxSize) break;

            memcpy(&profiles[number], &profile[0], sizeof(libtf_InputProcessorProfile_t));
            memcpy(&profiles[number].clsid, &profile[0].clsid, sizeof(CLSID));
            memcpy(&profiles[number].guidProfile, &profile[0].guidProfile, sizeof(GUID));
            memcpy(&profiles[number].catid, &profile[0].catid, sizeof(GUID));
            memcpy(&profiles[number].hklSubstitute, &profile[0].hklSubstitute, sizeof(HKL));
            memcpy(&profiles[number].hkl, &profile[0].hkl, sizeof(HKL));
        }

        number++;
    }
    *fetched = number;

    return S_OK;
}

/**
 * @brief Get active input processor profie of current thread
 */
HRESULT libtf_get_active_input_processor(libtf_InputProcessorProfile_t *profile)
{
    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(inputProcessorProfiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER));
    CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

    CHECK_HR(inputProcessorMgr->GetActiveProfile(GUID_TFCAT_TIP_KEYBOARD, profile));

    return S_OK;
}

/**
 * @brief Set active input processor profie of current thread
 */
HRESULT libtf_set_active_input_processor(libtf_InputProcessorProfile_t profile)
{
    CComPtr<ITfInputProcessorProfiles> inputProcessorProfiles;
    CHECK_HR(inputProcessorProfiles.CoCreateInstance(CLSID_TF_InputProcessorProfiles, NULL, CLSCTX_INPROC_SERVER));
    CComQIPtr<ITfInputProcessorProfileMgr> inputProcessorMgr = inputProcessorProfiles;

    CHECK_HR(inputProcessorMgr->ActivateProfile(profile.dwProfileType,
                                                profile.langid,
                                                profile.clsid,
                                                profile.guidProfile,
                                                profile.hkl,
                                                TF_IPPMF_DONTCARECURRENTINPUTLANGUAGE));

    return S_OK;
}
#pragma endregion

#pragma region Context
/**
 * @brief Create input context on calling thread
 */
HRESULT libtf_create_ctx(libtf_pInputContext *ctx)
{
    auto context = new InputContext_t();
    *ctx = context;

    context->ctx = new CInputContext();
    return context->ctx->initialize();
}

/**
 * @brief Dispose input context
 */
HRESULT libtf_dispose_ctx(libtf_pInputContext ctx)
{
    HRESULT hr = ctx->ctx->dispose();
    delete ctx;
    return hr;
}

/**
 * @brief Terminate all the compositions in the context
 */
HRESULT libtf_terminate_composition(libtf_pInputContext ctx)
{
    return ctx->ctx->terminateComposition();
}

/**
 * @brief Set input method state
 */
HRESULT libtf_set_im_state(libtf_pInputContext ctx, bool enable)
{
    return ctx->ctx->setIMState(enable);
}

/**
 * @brief Get input method state
 * 
 * @return true IM has enabled
 * @return false IM has disabled
 */
HRESULT libtf_get_im_state(libtf_pInputContext ctx, bool *imState)
{
    return ctx->ctx->getIMState(imState);
}

/**
 * @brief Set current focused window
 * 
 * @param hWnd window who receive WM_SETFOCUS on its message queue
 *             this parameter can be NULL if the context does not have the corresponding handle to the window.
 */
HRESULT libtf_set_focus_wnd(libtf_pInputContext ctx, HWND hWnd)
{
    return ctx->ctx->setFocus(hWnd);
}

/**
 * @brief Get current focused window
 */
HRESULT libtf_get_focus_wnd(libtf_pInputContext ctx, HWND *hWnd)
{
    *hWnd = ctx->hWnd;
    return S_OK;
}

/**
 * @brief Set Conversion Mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_conversion_mode(libtf_pInputContext ctx, libtf_ConversionMode mode)
{
    return ctx->ctx->m_conversionHander->setConversionMode(mode);
}

/**
 * @brief Set Sentence Mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_sentence_mode(libtf_pInputContext ctx, libtf_SentenceMode mode)
{
    return ctx->ctx->m_sentenceHander->setSentenceMode(mode);
}

/**
 * @brief Set if in Full Screen mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_full_screen(libtf_pInputContext ctx, bool isFullScreen)
{
    ctx->ctx->m_fullScHandler->m_isFullScreen = isFullScreen;
    return S_OK;
}

/**
 * @brief Set if input method should show its Candidate Window
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_show_candidate_list_wnd(libtf_pInputContext ctx, bool show)
{
    ctx->ctx->m_candHandler->m_showIMCandidateListWindow = show;
    return S_OK;
}
#pragma endregion

#pragma region setCallback
/**
 * @brief Set Composition Callback
 */
HRESULT libtf_set_composition_callback(libtf_pInputContext ctx, libtf_CallbackComposition callback)
{
    ctx->ctx->m_compositionHandler->m_sigComposition = callback;
    return S_OK;
}

/**
 * @brief Set Commit Callback
 */
HRESULT libtf_set_commit_callback(libtf_pInputContext ctx, libtf_CallbackCommit callback)
{
    ctx->ctx->m_compositionHandler->m_commitHandler->m_sigCommit = callback;
    return S_OK;
}

/**
 * @brief Set PreEdit Bounding Box Callback
 */
HRESULT libtf_set_bounding_box_callback(libtf_pInputContext ctx, libtf_CallbackBoundingBox callback)
{
    ctx->ctx->m_compositionHandler->m_sigBoundingBox = callback;
    return S_OK;
}

/**
 * @brief Set Candidate List Callback
 */
HRESULT libtf_set_candidate_list_callback(libtf_pInputContext ctx, libtf_CallbackCandidateList callback)
{
    ctx->ctx->m_candHandler->m_sigCandidateList = callback;
    return S_OK;
}

/**
 * @brief Set Conversion mode Callback
 */
HRESULT libtf_set_conversion_mode_callback(libtf_pInputContext ctx, libtf_CallbackConversionMode callback)
{
    ctx->ctx->m_conversionHander->sigConversionMode = callback;
    return S_OK;
}

/**
 * @brief Set Sentence mode Callback
 */
HRESULT libtf_set_sentence_mode_callback(libtf_pInputContext ctx, libtf_CallbackSentenceMode callback)
{
    ctx->ctx->m_sentenceHander->sigSentenceMode = callback;
    return S_OK;
}

/**
 * @brief Set Input Processor Callback
 */
HRESULT libtf_set_input_processor_callback(libtf_pInputContext ctx, libtf_CallbackInputProcessor callback)
{
    ctx->ctx->m_inputProcessor->sigInputProcessor = callback;
    return S_OK;
}
#pragma endregion