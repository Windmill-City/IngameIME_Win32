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
#ifdef USE_TfThread
    context->tfThread = new TfThread();
    AttachThreadInput(GetCurrentThreadId(), context->tfThread->getId(), true);
    auto future = context->tfThread->enqueue(
        [context]()
        {
            return context->ctx->initialize();
        });
    return future.get();
#else
    CoInitialize(NULL);
    return context->ctx->initialize();
#endif
}

/**
 * @brief Dispose input context
 */
HRESULT libtf_dispose_ctx(libtf_pInputContext ctx)
{
#ifdef USE_TfThread
    AttachThreadInput(GetCurrentThreadId(), ctx->tfThread->getId(), false);
    auto future = ctx->tfThread->enqueue(
        [ctx]()
        {
            return ctx->ctx->dispose();
        });
    HRESULT hr = future.get();
    delete ctx;
    return hr;
#else
    HRESULT hr = ctx->ctx->dispose();
    delete ctx;
    return hr;
#endif
}

/**
 * @brief Terminate all the compositions in the context
 */
HRESULT libtf_terminate_composition(libtf_pInputContext ctx)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx]()
        {
            return ctx->ctx->terminateComposition();
        });
    return future.get();
#else
    return ctx->ctx->terminateComposition();
#endif
}

/**
 * @brief Set input method state
 */
HRESULT libtf_set_im_state(libtf_pInputContext ctx, bool enable)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](bool enable)
        {
            return ctx->ctx->setIMState(enable);
        },
        enable);
    return future.get();
#else
    return ctx->ctx->setIMState(enable);
#endif
}

/**
 * @brief Get input method state
 * 
 * @return true IM has enabled
 * @return false IM has disabled
 */
HRESULT libtf_get_im_state(libtf_pInputContext ctx, bool *imState)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](bool *imState)
        {
            return ctx->ctx->getIMState(imState);
        },
        imState);
    return future.get();
#else
    return ctx->ctx->getIMState(imState);
#endif
}

/**
 * @brief Set current focused window
 * 
 * @param hWnd window who receive WM_SETFOCUS on its message queue
 *             this parameter can be NULL if the context does not have the corresponding handle to the window.
 */
HRESULT libtf_set_focus_wnd(libtf_pInputContext ctx, HWND hWnd)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](HWND hWnd)
        {
            ctx->hWnd = hWnd;
            return ctx->ctx->setFocus(hWnd);
        },
        hWnd);
    return future.get();
#else
    return ctx->ctx->setFocus(hWnd);
#endif
}

/**
 * @brief Get current focused window
 */
HRESULT libtf_get_focus_wnd(libtf_pInputContext ctx, HWND *hWnd)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](HWND *hWnd)
        {
            *hWnd = ctx->hWnd;
            return S_OK;
        },
        hWnd);
    return future.get();
#else
    *hWnd = ctx->hWnd;
    return S_OK;
#endif
}

/**
 * @brief Set Conversion Mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_conversion_mode(libtf_pInputContext ctx, ConversionMode mode)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](ConversionMode mode)
        {
            return ctx->ctx->m_conversionHander->setConversionMode(mode);
        },
        mode);
    return future.get();
#else
    return ctx->ctx->m_conversionHander->setConversionMode(mode);
#endif
}

/**
 * @brief Set Sentence Mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_sentence_mode(libtf_pInputContext ctx, SentenceMode mode)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](SentenceMode mode)
        {
            return ctx->ctx->m_sentenceHander->setSentenceMode(mode);
        },
        mode);
    return future.get();
#else
    return ctx->ctx->m_sentenceHander->setSentenceMode(mode);
#endif
}

/**
 * @brief Set if in Full Screen mode
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_full_screen(libtf_pInputContext ctx, bool isFullScreen)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](bool isFullScreen)
        {
            ctx->ctx->m_fullScHandler->m_isFullScreen = isFullScreen;
            return S_OK;
        },
        isFullScreen);
    return future.get();
#else
    ctx->ctx->m_fullScHandler->m_isFullScreen = isFullScreen;
    return S_OK;
#endif
}

/**
 * @brief Set if input method should show its Candidate Window
 * 
 * @return HRESULT 
 */
HRESULT libtf_set_show_candidate_list_wnd(libtf_pInputContext ctx, bool show)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](bool show)
        {
            ctx->ctx->m_candHandler->m_showIMCandidateListWindow = show;
            return S_OK;
        },
        show);
    return future.get();
#else
    ctx->ctx->m_candHandler->m_showIMCandidateListWindow = show;
    return S_OK;
#endif
}

#pragma region setCallback
/**
 * @brief Set Composition Callback
 */
HRESULT libtf_set_composition_callback(libtf_pInputContext ctx, CallbackComposition callback)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackComposition callback)
        {
            ctx->ctx->m_compHandler->m_sigComposition = callback;
            return S_OK;
        },
        callback);
    return future.get();
#else
    ctx->ctx->m_compHandler->m_sigComposition = callback;
    return S_OK;
#endif
}

/**
 * @brief Set PreEdit Bounding Box Callback
 */
HRESULT libtf_set_bounding_box_callback(libtf_pInputContext ctx, CallbackBoundingBox callback)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackBoundingBox callback)
        {
            ctx->ctx->m_compHandler->m_sigBoundingBox = callback;
            return S_OK;
        },
        callback);
    return future.get();
#else
    ctx->ctx->m_compHandler->m_sigBoundingBox = callback;
    return S_OK;
#endif
}

/**
 * @brief Set Candidate List Callback
 */
HRESULT libtf_set_candidate_list_callback(libtf_pInputContext ctx, CallbackCandidateList callback)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackCandidateList callback)
        {
            ctx->ctx->m_candHandler->m_sigCandidateList = callback;
            return S_OK;
        },
        callback);
    return future.get();
#else
    ctx->ctx->m_candHandler->m_sigCandidateList = callback;
    return S_OK;
#endif
}

/**
 * @brief Set Conversion mode Callback
 */
HRESULT libtf_set_conversion_mode_callback(libtf_pInputContext ctx, CallbackConversionMode callback)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackConversionMode callback)
        {
            ctx->ctx->m_conversionHander->sigConversionMode = callback;
            return S_OK;
        },
        callback);
    return future.get();
#else
    ctx->ctx->m_conversionHander->sigConversionMode = callback;
    return S_OK;
#endif
}

/**
 * @brief Set Sentence mode Callback
 */
HRESULT libtf_set_sentence_mode_callback(libtf_pInputContext ctx, CallbackSentenceMode callback)
{
#ifdef USE_TfThread
    auto future = ctx->tfThread->enqueue(
        [ctx](CallbackSentenceMode callback)
        {
            ctx->ctx->m_sentenceHander->sigSentenceMode = callback;
            return S_OK;
        },
        callback);
    return future.get();
#else
    ctx->ctx->m_sentenceHander->sigSentenceMode = callback;
    return S_OK;
#endif
}
#pragma endregion