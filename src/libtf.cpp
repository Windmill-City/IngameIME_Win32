#include <algorithm>
#include <map>
#include <numeric>

#include "InputContext.hpp"
#include "libtf.h"

#pragma region InputContext
typedef struct libtf_InputContext
{
    CComPtr<libtf::CInputContext> m_InputContext;
    // Candidate List
    libtf_CandidateListCallback m_PrevCandidateListCallback = NULL;
    void*                       m_PrevCandidateListUserData = NULL;
    // PreEdit
    libtf_PreEditCallback m_PrevPreEditCallback = NULL;
    void*                 m_PrevPreEditUserData = NULL;
    // PreEdit Rect
    libtf_PreEditRectCallback m_PrevPreEditRectCallback = NULL;
    void*                     m_PrevPreEditRectUserData = NULL;
    // Commit
    libtf_CommitCallback m_PrevCommitCallback = NULL;
    void*                m_PrevCommitUserData = NULL;
    // InputProcessor
    libtf_InputProcessorCallback m_PrevInputProcessorCallback = NULL;
    void*                        m_PrevInputProcessorUserData = NULL;
} libtf_InputContext_t;
/**
 * @brief Create input context for specific window
 *
 * @param hWnd handle of the window to create the context
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the window
 */
LIBTF_EXPORT HRESULT libtf_create_ctx(libtf_pInputContext* ctx, const HWND hWnd)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    auto inputCtx = new libtf_InputContext_t();
    *ctx          = inputCtx;

    inputCtx->m_InputContext = new libtf::CInputContext();

    BEGIN_HRESULT_SCOPE();

    CHECK_HR(inputCtx->m_InputContext->ctor(hWnd));

    return hr;
    END_HRESULT_SCOPE();

    delete inputCtx;
    *ctx = NULL;

    END_HRESULT();
}
/**
 * @brief Cleanup input context
 *
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_destroy_ctx(libtf_pInputContext* ctx)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR((*ctx)->m_InputContext->dtor());

    delete *ctx;
    *ctx = NULL;

    END_HRESULT();
}
/**
 * @brief Set if context activated
 *
 * @param activated set to true to activate input method
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_set_activated(libtf_pInputContext ctx, const bool activated)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR(ctx->m_InputContext->setActivated(activated));

    END_HRESULT();
}
/**
 * @brief Get context activate state
 *
 * @param activated receive context activate state
 * @return E_INVALIDARG if activated is NULL
 */
LIBTF_EXPORT HRESULT libtf_get_activated(const libtf_pInputContext ctx, bool* activated)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR(ctx->m_InputContext->getActivated(activated));

    END_HRESULT();
}
#pragma endregion
#pragma region FullScreenHandler
/**
 * @brief Set if the game is in fullScreen mode
 *
 * @param fullScreen is game window in fullScreen mode?
 * @return HRESULT
 */
LIBTF_EXPORT HRESULT libtf_set_fullscreen(libtf_pInputContext ctx, const bool fullscreen)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR(ctx->m_InputContext->m_FullScreenUIElementHandler->setFullScreen(fullscreen));

    END_HRESULT();
}
/**
 * @brief Get context fullScreen state
 *
 * @param fullScreen receive fullScreen state
 * @return E_INVALIDARG if fullScreen is NULL
 */
LIBTF_EXPORT HRESULT libtf_get_fullscreen(const libtf_pInputContext ctx, bool* fullscreen)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR(ctx->m_InputContext->m_FullScreenUIElementHandler->getFullScreen(fullscreen));

    END_HRESULT();
}
#pragma endregion
#pragma region CandidateListHandler
/**
 * @brief Set CandidateList callback to receive candidate list data in fullscreen mode
 *
 * @param usetData Previous userData will send out here
 * @return Previous callback, Nullable
 */
LIBTF_EXPORT libtf_CandidateListCallback libtf_candidate_list_set_callback(libtf_pInputContext         ctx,
                                                                           libtf_CandidateListCallback callback,
                                                                           void*                       userData)
{
    if (!ctx) return NULL;

    auto prev                        = ctx->m_PrevCandidateListCallback;
    ctx->m_PrevCandidateListCallback = callback;

    if (callback)
        ctx->m_InputContext->m_FullScreenUIElementHandler->m_CandidateListHandler->setCallback(
            [callback, userData](auto&& state, auto&& ctx) {
                // NULL context only state update
                if (!ctx) {
                    callback(state, NULL, userData);
                    return;
                }

                auto strTotalLen = std::accumulate(ctx->m_Candidates.begin(),
                                                   ctx->m_Candidates.end(),
                                                   ctx->m_PageSize,// Number of \0
                                                   [](auto&& sum, auto&& it) { return sum + (uint32_t)it.length(); });
                auto libtf_ctx   = std::unique_ptr<libtf_CandidateListContext_t, decltype(&::free)>(
                    (libtf_pCandidateListContext)malloc(sizeof(libtf_CandidateListContext_t) +
                                                        sizeof(wchar_t*) * ctx->m_PageSize +
                                                        sizeof(wchar_t) * strTotalLen),
                    free);

                // Out of memory
                if (!libtf_ctx) return;

                libtf_ctx->m_Selection = ctx->m_Selection;
                libtf_ctx->m_PageSize  = ctx->m_PageSize;

                // Append strings at the end of the struct
                int  i           = 0;
                auto pCandidates = (wchar_t*)&libtf_ctx->m_Candidates[ctx->m_PageSize];
                for (auto&& it : ctx->m_Candidates) {
                    // Point to string start
                    libtf_ctx->m_Candidates[i++] = pCandidates;

                    auto size = it.length() + 1;
                    // Copy string data
                    memcpy(pCandidates, it.c_str(), size * sizeof(wchar_t));
                    pCandidates += size;
                }

                callback(state, libtf_ctx.get(), userData);
            });
    else
        ctx->m_InputContext->m_FullScreenUIElementHandler->m_CandidateListHandler->setCallback(NULL);

    std::swap(ctx->m_PrevCandidateListUserData, userData);

    return prev;
}
/**
 * @brief Set currently selected Candidate
 * When user press Space key, input method will commit with currently selected Candidate String
 *
 * @param index Candidate's index
 * @return HRESULT
 */
LIBTF_EXPORT HRESULT libtf_candidate_list_set_sel(const libtf_pInputContext ctx, uint32_t index)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    auto candCtx = ctx->m_InputContext->m_FullScreenUIElementHandler->m_CandidateListHandler->m_Context;
    if (candCtx) CHECK_HR(candCtx->select(index));

    END_HRESULT();
}
/**
 * @brief Force input method to commit with currently selected Candidate String
 *
 * @return HRESULT
 */
LIBTF_EXPORT HRESULT libtf_candidate_list_finalize(const libtf_pInputContext ctx)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    auto candCtx = ctx->m_InputContext->m_FullScreenUIElementHandler->m_CandidateListHandler->m_Context;
    if (candCtx) CHECK_HR(candCtx->finalize());

    END_HRESULT();
}
#pragma endregion
#pragma region CompositionHandler
/**
 * @brief Terminate active composition
 *
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_composition_terminate(const libtf_pInputContext ctx)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR(ctx->m_InputContext->m_CompositionHandler->terminate());

    END_HRESULT();
}
#pragma endregion
#pragma region PreEditHandler
/**
 * @brief Set PreEdit callback to receive preedit and compostion state
 *
 * @param usetData Previous userData will send out here
 * @return Previous callback, Nullable
 */
LIBTF_EXPORT libtf_PreEditCallback libtf_preedit_set_callback(libtf_pInputContext   ctx,
                                                              libtf_PreEditCallback callback,
                                                              void*                 userData)
{
    if (!ctx) return NULL;

    auto prev                  = ctx->m_PrevPreEditCallback;
    ctx->m_PrevPreEditCallback = callback;

    if (callback)
        ctx->m_InputContext->m_CompositionHandler->m_PreEditHandler->libtf::PreEditContextCallback::setCallback(
            [callback, userData](auto&& state, auto&& ctx) {
                // NULL context only state update
                if (!ctx) {
                    callback(state, NULL, userData);
                    return;
                }

                auto strTotalLen = ctx->m_Content.length() + 1;
                auto libtf_ctx   = std::unique_ptr<libtf_PreEditContext_t, decltype(&::free)>(
                    (libtf_pPreEditContext)malloc(sizeof(libtf_PreEditContext_t) + sizeof(wchar_t) * strTotalLen),
                    free);

                // Out of memory
                if (!libtf_ctx) return;

                libtf_ctx->m_SelStart = ctx->m_SelStart;
                libtf_ctx->m_SelEnd   = ctx->m_SelEnd;

                // Append string at the end of the struct
                auto pContent = libtf_ctx->m_Content;

                auto size = (ctx->m_Content.length() + 1) * sizeof(wchar_t);
                // Copy string data
                memcpy(pContent, ctx->m_Content.c_str(), size);

                callback(state, libtf_ctx.get(), userData);
            });
    else
        ctx->m_InputContext->m_CompositionHandler->m_PreEditHandler->libtf::PreEditContextCallback::setCallback(NULL);

    std::swap(ctx->m_PrevPreEditUserData, userData);

    return prev;
}
/**
 * @brief Set PreEdit Rect callback to position input method's candidatelist window
 *
 * @param usetData Previous userData will send out here
 * @return Previous callback, Nullable
 *
 * @note If the length of preedit is 0 (as it would be drawn by input method), the rectangle
 * coincides with the insertion point, and its width is 0.
 */
LIBTF_EXPORT libtf_PreEditRectCallback libtf_preedit_rect_set_callback(libtf_pInputContext       ctx,
                                                                       libtf_PreEditRectCallback callback,
                                                                       void*                     userData)
{
    if (!ctx) return NULL;

    auto prev                      = ctx->m_PrevPreEditRectCallback;
    ctx->m_PrevPreEditRectCallback = callback;

    if (callback)
        ctx->m_InputContext->m_CompositionHandler->m_PreEditHandler->libtf::PreEditRectCallback::setCallback(
            [callback, userData](auto&& it) { callback(it, userData); });
    else
        ctx->m_InputContext->m_CompositionHandler->m_PreEditHandler->libtf::PreEditRectCallback::setCallback(NULL);

    std::swap(ctx->m_PrevPreEditRectUserData, userData);

    return prev;
}
#pragma endregion
#pragma region CommitHandler
/**
 * @brief Set Commit callback to receive commit string
 */
LIBTF_EXPORT libtf_CommitCallback libtf_commit_set_callback(libtf_pInputContext  ctx,
                                                            libtf_CommitCallback callback,
                                                            void*                userData)
{
    if (!ctx) return NULL;

    auto prev                 = ctx->m_PrevCommitCallback;
    ctx->m_PrevCommitCallback = callback;

    if (callback)
        ctx->m_InputContext->m_CompositionHandler->m_CommitHandler->setCallback(
            [callback, userData](auto&& it) { callback(it.c_str(), userData); });
    else
        ctx->m_InputContext->m_CompositionHandler->m_CommitHandler->setCallback(NULL);

    std::swap(ctx->m_PrevCommitUserData, userData);

    return prev;
}
#pragma endregion
#pragma region InputProcessor
/**
 * @brief Convert libtf::InputProcessorContext to libtf_InputProcessorContext_t
 *
 * @param processorCtx libtf::InputProcessorContext
 * @return NULL if out of memory
 */
libtf_pInputProcessorContext inputprocessor_get_ctx(const libtf::InputProcessorContext processorCtx)
{
    auto strTotalLen = std::accumulate(processorCtx.m_InputModes.begin(),
                                       processorCtx.m_InputModes.end(),
                                       processorCtx.m_InputModeSize,// Number of \0
                                       [](auto&& sum, auto&& it) { return sum + (uint32_t)it.length(); });
    auto libtf_ctx   = (libtf_pInputProcessorContext)malloc(sizeof(libtf_InputProcessorContext_t) +
                                                          sizeof(wchar_t*) * processorCtx.m_InputModeSize +
                                                          sizeof(wchar_t) * strTotalLen);

    // Out of memory
    if (!libtf_ctx) return NULL;

    libtf_ctx->m_InputProcessor = (void*)processorCtx.m_InputProcessor.get();
    libtf_ctx->m_InputModeSize  = processorCtx.m_InputModeSize;

    // Append strings at the end of the struct
    int  i           = 0;
    auto pInputModes = (wchar_t*)&libtf_ctx->m_InputModes[libtf_ctx->m_InputModeSize];

    for (auto&& it : processorCtx.m_InputModes) {
        // Point to string start
        libtf_ctx->m_InputModes[i++] = pInputModes;

        auto size = it.length() + 1;
        // Copy string data
        memcpy(pInputModes, it.c_str(), size * sizeof(wchar_t));
        pInputModes += size;
    }

    return libtf_ctx;
}
/**
 * @brief Set InputProcessor callback to receive InputProcessor relevent event
 *
 * @param usetData Previous userData will send out here
 * @return Previous callback, Nullable
 */
LIBTF_EXPORT libtf_InputProcessorCallback libtf_inputprocessor_set_callback(libtf_pInputContext          ctx,
                                                                            libtf_InputProcessorCallback callback,
                                                                            void*                        userData)
{
    if (!ctx) return NULL;

    auto prev                         = ctx->m_PrevInputProcessorCallback;
    ctx->m_PrevInputProcessorCallback = callback;

    if (callback)
        ctx->m_InputContext->m_InputProcessorHandler->setCallback([callback, userData](auto&& state, auto&& ctx) {
            auto libtf_ctx =
                std::unique_ptr<libtf_InputProcessorContext_t, decltype(&::free)>(inputprocessor_get_ctx(*ctx), free);
            if (libtf_ctx) callback(state, libtf_ctx.get(), userData);
        });
    else
        ctx->m_InputContext->m_InputProcessorHandler->setCallback(NULL);

    std::swap(ctx->m_PrevInputProcessorUserData, userData);

    return prev;
}
/**
 * @brief Get active InputProcessor context
 *
 * @return NULL if out of memory
 * @note You need to free it using libtf_inputprocessor_free_ctx
 */
LIBTF_EXPORT libtf_pInputProcessorContext libtf_inputprocessor_get_ctx(const libtf_pInputContext ctx)
{
    if (!ctx) return NULL;

    auto processorCtx = ctx->m_InputContext->m_InputProcessorHandler->m_Context;
    return inputprocessor_get_ctx(*processorCtx);
}
/**
 * @brief Free InputProcessor context
 */
LIBTF_EXPORT void libtf_inputprocessor_free_ctx(libtf_pInputProcessorContext* ctx)
{
    if (ctx && *ctx) {
        free(*ctx);
        *ctx = NULL;
    }
}
/**
 * @brief Apply new InputMode to active InputProcessor
 *
 * @param newMode New InputMode, Zero-terminated string
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_inputprocessor_apply_inputmode(libtf_pInputContext ctx, const wchar_t* newMode)
{
    BEGIN_HRESULT();

    if (!ctx) return E_INVALIDARG;

    CHECK_HR(ctx->m_InputContext->m_InputProcessorHandler->applyInputMode(newMode));

    END_HRESULT();
}
/**
 * @brief Get InputProcessors available
 *
 * @return Empty list if the calling thread is not a UI Thread
 * @return NULL if out of memory
 * @note You need to free it using libtf_free_inputprocessors
 */
LIBTF_EXPORT libtf_pInputProcessors libtf_get_inputprocessors()
{
    try {
        auto processors = libtf::InputProcessor::getInputProcessors();

        auto libtf_processors = (libtf_pInputProcessors)malloc(sizeof(libtf_InputProcessors_t) +
                                                               processors.size() * sizeof(libtf_HInputProcessor));

        if (!libtf_processors) return NULL;

        libtf_processors->m_InputProcessorsSize = (int)processors.size();

        int i = 0;
        for (auto&& it : processors) { libtf_processors->m_InputProcessors[i++] = (void*)it.get(); }

        return libtf_processors;
    }
    catch (std::bad_alloc) {
        return NULL;
    }
}
/**
 * @brief Free the libtf_InputProcessors object
 */
LIBTF_EXPORT void libtf_free_inputprocessors(libtf_pInputProcessors* processors)
{
    if (processors && *processors) {
        free(*processors);
        *processors = NULL;
    }
}
/**
 * @brief Get detailed infomation of the InputProcessor
 *
 * @note You need to free it using libtf_inputprocessor_free_profile
 */
LIBTF_EXPORT libtf_pInputProcessorProfile libtf_inputprocessor_get_profile(const libtf_HInputProcessor hInputProcessor)
{
    if (!hInputProcessor) return NULL;

    auto processor   = ((const libtf::InputProcessor*)hInputProcessor);
    auto strTotalLen = processor->m_InputProcessorName.length();
    strTotalLen += processor->m_Locale.length();
    strTotalLen += processor->m_LocaleName.length();
    strTotalLen += 3;// Number of \0

    auto libtf_profile =
        (libtf_pInputProcessorProfile)malloc(sizeof(libtf_InputProcessorProfile_t) + sizeof(wchar_t) * strTotalLen);

    // Out of memory
    if (!libtf_profile) return NULL;

    libtf_profile->m_Type = processor->m_Type;

    // Append string at the end of the struct
    auto pString = (wchar_t*)((uint8_t*)libtf_profile + sizeof(libtf_InputProcessorProfile_t));

    // Locale
    libtf_profile->m_Locale = pString;

    int size = (int)((processor->m_Locale.length() + 1));
    memcpy(pString, processor->m_Locale.c_str(), size * sizeof(wchar_t));
    pString += size;

    // Locale Name
    libtf_profile->m_LocaleName = pString;

    size = (int)((processor->m_LocaleName.length() + 1));
    memcpy(pString, processor->m_LocaleName.c_str(), size * sizeof(wchar_t));
    pString += size;

    // ProcessorName
    libtf_profile->m_InputProcessorName = pString;

    size = (int)((processor->m_InputProcessorName.length() + 1));
    memcpy(pString, processor->m_InputProcessorName.c_str(), size * sizeof(wchar_t));

    return libtf_profile;
}
/**
 * @brief Free the libtf_InputProcessorProfile
 */
LIBTF_EXPORT void libtf_inputprocessor_free_profile(libtf_pInputProcessorProfile* profile)
{
    if (profile && *profile) {
        free(*profile);
        *profile = NULL;
    }
}
/**
 * @brief Set active InputProcessor for calling thread
 *
 * @return UI_E_WRONG_THREAD if the calling thread is not a UI Thread
 */
LIBTF_EXPORT HRESULT libtf_inputprocessor_set_activated(const libtf_HInputProcessor hInputProcessor)
{
    BEGIN_HRESULT();

    if (!hInputProcessor) return E_INVALIDARG;

    CHECK_HR(((libtf::InputProcessor*)hInputProcessor)->setActivated());

    END_HRESULT();
}
#pragma endregion