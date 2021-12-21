#pragma once
#include "libtfdef.h"

#ifdef __cplusplus
extern "C" {
#endif

#pragma region InputContext
/**
 * @brief Create input context for specific window
 *
 * @param hWnd handle of the window to create the context
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the window
 */
LIBTF_EXPORT HRESULT libtf_create_ctx(libtf_pInputContext* ctx, const HWND hWnd);
/**
 * @brief Cleanup input context
 *
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_destroy_ctx(libtf_pInputContext* ctx);
/**
 * @brief Set if context activated
 *
 * @param activated set to true to activate input method
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_set_activated(libtf_pInputContext ctx, const bool activated);
/**
 * @brief Get context activate state
 *
 * @param activated receive context activate state
 * @return E_INVALIDARG if activated is NULL
 */
LIBTF_EXPORT HRESULT libtf_get_activated(const libtf_pInputContext ctx, bool* activated);
#pragma endregion
#pragma region FullScreenHandler
/**
 * @brief Set if the game is in fullScreen mode
 *
 * @param fullScreen is game window in fullScreen mode?
 * @return HRESULT
 */
LIBTF_EXPORT HRESULT libtf_set_fullscreen(libtf_pInputContext ctx, const bool fullscreen);
/**
 * @brief Get context fullScreen state
 *
 * @param fullScreen receive fullScreen state
 * @return E_INVALIDARG if fullScreen is NULL
 */
LIBTF_EXPORT HRESULT libtf_get_fullscreen(const libtf_pInputContext ctx, bool* fullscreen);
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
                                                                           void*                       userData);
/**
 * @brief Set currently selected Candidate
 * When user press Space key, input method will commit with currently selected Candidate String
 *
 * @param index Candidate's index
 * @return HRESULT
 */
LIBTF_EXPORT HRESULT libtf_candidate_list_set_sel(const libtf_pInputContext ctx, uint32_t index);
/**
 * @brief Force input method to commit with currently selected Candidate String
 *
 * @return HRESULT
 */
LIBTF_EXPORT HRESULT libtf_candidate_list_finalize(const libtf_pInputContext ctx);
#pragma endregion
#pragma region CompositionHandler
/**
 * @brief Terminate active composition
 *
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_composition_terminate(const libtf_pInputContext ctx);
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
                                                              void*                 userData);
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
                                                                       void*                     userData);
#pragma endregion
#pragma region CommitHandler
/**
 * @brief Set Commit callback to receive commit string
 *
 * @param usetData Previous userData will send out here
 * @return Previous callback, Nullable
 */
LIBTF_EXPORT libtf_CommitCallback libtf_commit_set_callback(libtf_pInputContext  ctx,
                                                            libtf_CommitCallback callback,
                                                            void*                userData);
#pragma endregion
#pragma region InputProcessor
/**
 * @brief Set InputProcessor callback to receive InputProcessor relevent event
 *
 * @param usetData Previous userData will send out here
 * @return Previous callback, Nullable
 */
LIBTF_EXPORT libtf_InputProcessorCallback libtf_inputprocessor_set_callback(libtf_pInputContext          ctx,
                                                                            libtf_InputProcessorCallback callback,
                                                                            void*                        userData);
/**
 * @brief Get active InputProcessor context
 *
 * @return NULL if out of memory
 * @note You need to free it using libtf_inputprocessor_free_ctx
 */
LIBTF_EXPORT libtf_pInputProcessorContext libtf_inputprocessor_get_ctx(const libtf_pInputContext ctx);
/**
 * @brief Free InputProcessor context
 */
LIBTF_EXPORT void libtf_inputprocessor_free_ctx(libtf_pInputProcessorContext* ctx);
/**
 * @brief Apply new InputMode to active InputProcessor
 *
 * @param newMode New InputMode, Zero-terminated string
 * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
 */
LIBTF_EXPORT HRESULT libtf_inputprocessor_apply_inputmode(libtf_pInputContext ctx, const wchar_t* newMode);
/**
 * @brief Get InputProcessors available
 *
 * @return Empty list if the calling thread is not a UI Thread
 * @return NULL if out of memory
 * @note You need to free it using libtf_free_inputprocessors
 */
LIBTF_EXPORT libtf_pInputProcessors libtf_get_inputprocessors();
/**
 * @brief Free the libtf_InputProcessors object
 */
LIBTF_EXPORT void libtf_free_inputprocessors(libtf_pInputProcessors* processors);
/**
 * @brief Get detailed infomation of the InputProcessor
 *
 * @return NULL if out of memory
 * @note You need to free it using libtf_inputprocessor_free_profile
 */
LIBTF_EXPORT libtf_pInputProcessorProfile libtf_inputprocessor_get_profile(const libtf_HInputProcessor hInputProcessor);
/**
 * @brief Free the libtf_InputProcessorProfile
 */
LIBTF_EXPORT void libtf_inputprocessor_free_profile(libtf_pInputProcessorProfile* profile);
/**
 * @brief Set active InputProcessor for calling thread
 *
 * @return UI_E_WRONG_THREAD if the calling thread is not a UI Thread
 */
LIBTF_EXPORT HRESULT libtf_inputprocessor_set_activated(const libtf_HInputProcessor hInputProcessor);
#pragma endregion

#ifdef __cplusplus
}
#endif