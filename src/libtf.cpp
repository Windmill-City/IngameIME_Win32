#include "libtf.h"

using namespace libtf;
/**
 * @brief Create input context on calling thread
 */
HRESULT create(pInputContext ctx)
{
    InputContext_t context;
    ctx = &context;

    ctx->ctx = new CInputContext();
    ctx->tfThread = new TfThread();
    AttachThreadInput(ctx->tfThread->getId(), GetCurrentThreadId(), true);
    auto future = ctx->tfThread->enqueue(
        [ctx]()
        {
            return ctx->ctx->initialize();
        });
    return future.get();
}

/**
 * @brief Dispose input context
 */
HRESULT dispose(pInputContext ctx)
{
    AttachThreadInput(ctx->tfThread->getId(), GetCurrentThreadId(), false);
    auto future = ctx->tfThread->enqueue(
        [ctx]()
        {
            return ctx->ctx->dispose();
        });
    return future.get();
}

/**
 * @brief Terminate all the compositions in the context
 */
HRESULT terminateComposition(pInputContext ctx)
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
HRESULT setIMState(pInputContext ctx, bool enable)
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
HRESULT getIMState(pInputContext ctx, bool *imState)
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
HRESULT setFocus(pInputContext ctx, HWND hWnd)
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
HRESULT getFocus(pInputContext ctx, HWND *hWnd)
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
HRESULT setConversionMode(pInputContext ctx, ConversionMode mode)
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
HRESULT setSentenceMode(pInputContext ctx, SentenceMode mode)
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
HRESULT setFullScreen(pInputContext ctx, bool isFullScreen)
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
HRESULT setShowCandidateWnd(pInputContext ctx, bool show)
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
HRESULT setCompositionCallback(pInputContext ctx, CallbackComposition callback)
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
HRESULT setBoundingBoxCallback(pInputContext ctx, CallbackBoundingBox callback)
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
HRESULT setCandidateListCallback(pInputContext ctx, CallbackCandidateList callback)
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
HRESULT setConversionModeCallback(pInputContext ctx, CallbackConversionMode callback)
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
HRESULT setSentenceModeCallback(pInputContext ctx, CallbackSentenceMode callback)
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