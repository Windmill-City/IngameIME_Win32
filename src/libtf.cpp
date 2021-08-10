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
    context->tfThread = new TfThread();
    AttachThreadInput(context->tfThread->getId(), GetCurrentThreadId(), true);
    auto future = context->tfThread->enqueue(
        [context]()
        {
            return context->ctx->initialize();
        });
    return future.get();
}

/**
 * @brief Dispose input context
 */
HRESULT libtf_dispose_ctx(libtf_pInputContext ctx)
{
    AttachThreadInput(ctx->tfThread->getId(), GetCurrentThreadId(), false);
    auto future = ctx->tfThread->enqueue(
        [ctx]()
        {
            return ctx->ctx->dispose();
        });
    HRESULT hr = future.get();
    delete ctx;
    return hr;
}

/**
 * @brief Terminate all the compositions in the context
 */
HRESULT libtf_terminate_composition(libtf_pInputContext ctx)
{
    auto future = ctx->tfThread->enqueue(
        [ctx]()
        {
            return ctx->ctx->terminateComposition();
        });
    return future.get();
}

/**
 * @brief Set input method state
 */
HRESULT libtf_set_im_state(libtf_pInputContext ctx, bool enable)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](bool enable)
        {
            return ctx->ctx->setIMState(enable);
        },
        enable);
    return future.get();
}

/**
 * @brief Get input method state
 * 
 * @return true IM has enabled
 * @return false IM has disabled
 */
HRESULT libtf_get_im_state(libtf_pInputContext ctx, bool *imState)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](bool *imState)
        {
            return ctx->ctx->getIMState(imState);
        },
        imState);
    return future.get();
}

/**
 * @brief Set current focused window
 * 
 * @param hWnd window who receive WM_SETFOCUS on its message queue
 *             this parameter can be NULL if the context does not have the corresponding handle to the window.
 */
HRESULT libtf_set_focus_wnd(libtf_pInputContext ctx, HWND hWnd)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](HWND hWnd)
        {
            ctx->hWnd = hWnd;
            return ctx->ctx->setFocus(hWnd);
        },
        hWnd);
    return future.get();
}

/**
 * @brief Get current focused window
 */
HRESULT libtf_get_focus_wnd(libtf_pInputContext ctx, HWND *hWnd)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](HWND *hWnd)
        {
            *hWnd = ctx->hWnd;
            return S_OK;
        },
        hWnd);
    return future.get();
}

/**
 * @brief Set Conversion Mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_conversion_mode(libtf_pInputContext ctx, ConversionMode mode)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](ConversionMode mode)
        {
            return ctx->ctx->m_conversionHander->setConversionMode(mode);
        },
        mode);
    return future.get();
}

/**
 * @brief Set Sentence Mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_sentence_mode(libtf_pInputContext ctx, SentenceMode mode)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](SentenceMode mode)
        {
            return ctx->ctx->m_sentenceHander->setSentenceMode(mode);
        },
        mode);
    return future.get();
}

/**
 * @brief Set if in Full Screen mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_full_screen(libtf_pInputContext ctx, bool isFullScreen)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](bool isFullScreen)
        {
            ctx->ctx->m_fullScHandler->m_isFullScreen = isFullScreen;
            return S_OK;
        },
        isFullScreen);
    return future.get();
}

/**
 * @brief Set if input method should show its Candidate Window
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_show_candidate_list_wnd(libtf_pInputContext ctx, bool show)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](bool show)
        {
            ctx->ctx->m_candHandler->m_showIMCandidateListWindow = show;
            return S_OK;
        },
        show);
    return future.get();
}

#pragma region setCallback
/**
 * @brief Set Composition Callback
 */
HRESULT libtf_set_composition_callback(libtf_pInputContext ctx, CallbackComposition callback)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackComposition callback)
        {
            ctx->ctx->m_compHandler->m_sigComposition = callback;
            return S_OK;
        },
        callback);
    return future.get();
}

/**
 * @brief Set PreEdit Bounding Box Callback
 */
HRESULT libtf_set_bounding_box_callback(libtf_pInputContext ctx, CallbackBoundingBox callback)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackBoundingBox callback)
        {
            ctx->ctx->m_compHandler->m_sigBoundingBox = callback;
            return S_OK;
        },
        callback);
    return future.get();
}

/**
 * @brief Set Candidate List Callback
 */
HRESULT libtf_set_candidate_list_callback(libtf_pInputContext ctx, CallbackCandidateList callback)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackCandidateList callback)
        {
            ctx->ctx->m_candHandler->m_sigCandidateList = callback;
            return S_OK;
        },
        callback);
    return future.get();
}

/**
 * @brief Set Conversion mode Callback
 */
HRESULT libtf_set_conversion_mode_callback(libtf_pInputContext ctx, CallbackConversionMode callback)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackConversionMode callback)
        {
            ctx->ctx->m_conversionHander->sigConversionMode = callback;
            return S_OK;
        },
        callback);
    return future.get();
}

/**
 * @brief Set Sentence mode Callback
 */
HRESULT libtf_set_sentence_mode_callback(libtf_pInputContext ctx, CallbackSentenceMode callback)
{
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackSentenceMode callback)
        {
            ctx->ctx->m_sentenceHander->sigSentenceMode = callback;
            return S_OK;
        },
        callback);
    return future.get();
}
#pragma endregion