#include "libtf.h"

using namespace libtf;
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
#pragma endregion