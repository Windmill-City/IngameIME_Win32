#pragma once
#include "libtfdef.h"

#include "CandidateListHandler.hpp"
#include "CompositionHandler.hpp"
#include "ConversionModeHandler.hpp"
#include "InputProcessorHandler.hpp"
#include "SentenceModeHandler.hpp"

namespace libtf {
    struct tagInputContext;
}// namespace libtf

extern "C" {
typedef struct libtf::tagInputContext*  libtf_pInputContext;
typedef struct TF_INPUTPROCESSORPROFILE libtf_InputProcessorProfile_t, *libtf_pInputProcessorProfile;

#pragma region InputProcesser Profile
/**
 * @brief Get available input processor profies for the calling thread
 *
 * @param profiles Pointer to an array of libtf_InputProcessorProfile_t.
 * This array must be at least maxSize elements in size
 * @param maxSize the max size of the profiles array
 * @param fetched if profiles is NULL, return the max count can obtain,
 * otherwise, return the count of elements actually obtained
 */
HRESULT libtf_get_input_processors(libtf_InputProcessorProfile_t* profiles, size_t maxSize, size_t* fetched);

/**
 * @brief Get active input processor profie for the calling thread
 */
HRESULT libtf_get_active_input_processor(libtf_InputProcessorProfile_t*);

/**
 * @brief Set active input processor profie for the calling thread
 */
HRESULT libtf_set_active_input_processor(libtf_InputProcessorProfile_t);
#pragma endregion

#pragma region Context
/**
 * @brief Create input context for the calling thread
 *
 * @note the following method in the Context region need to be call from the creator thread of the context,
 * if you are calling which from another thread, your call will be handled at the creator thread,
 * and the call will not return until the handling is finished, this may cause a dead lock!
 */
HRESULT libtf_create_ctx(libtf_pInputContext*);

/**
 * @brief Dispose the input context, the pointer to the context will be invailed
 */
HRESULT libtf_dispose_ctx(libtf_pInputContext);

/**
 * @brief Terminate active composition of the context
 */
HRESULT libtf_terminate_composition(libtf_pInputContext);

/**
 * @brief Set input method state of the context
 *
 * @param bool true to enable the input method, false to disable it
 */
HRESULT libtf_set_im_state(libtf_pInputContext, bool);

/**
 * @brief Get input method(IM) state of the context
 *
 * @param bool returns true if IM is enabled, false otherwise
 */
HRESULT libtf_get_im_state(libtf_pInputContext, bool*);

/**
 * @brief This method should be called from the WndProc of the ui-thread, of whom is the creator of this
 * context
 *
 * @param hWnd The window who receives the message
 * @param message can be one of WM_SETFOCUS/WM_KILLFOCUS
 */
HRESULT libtf_on_focus_msg(libtf_pInputContext, HWND hWnd, UINT message);

/**
 * @brief Get current focused window of the context
 *
 * @param HWND* current focused window, this can be NULL if no window get focused
 */
HRESULT libtf_get_focus_wnd(libtf_pInputContext, HWND*);

/**
 * @brief Set Conversion Mode of the context
 */
HRESULT libtf_set_conversion_mode(libtf_pInputContext, libtf_ConversionMode);

/**
 * @brief Get Sentence Mode of the context
 */
HRESULT libtf_get_sentence_mode(libtf_pInputContext, libtf_SentenceMode*);

/**
 * @brief Get Conversion Mode of the context
 */
HRESULT libtf_get_conversion_mode(libtf_pInputContext, libtf_ConversionMode*);

/**
 * @brief Set Sentence Mode of the context
 */
HRESULT libtf_set_sentence_mode(libtf_pInputContext, libtf_SentenceMode);

/**
 * @brief Set Full Screen mode of the context
 *
 * @note this call will not be handled in the creator thread of the context
 */
HRESULT libtf_set_full_screen(libtf_pInputContext, bool);

/**
 * @brief Set if input method should show its Candidate Window of the context
 *
 * @note this call will not be handled in the creator thread of the context
 */
HRESULT libtf_set_show_candidate_list_wnd(libtf_pInputContext, bool);

/**
 * @brief Get Full Screen mode of the context
 *
 * @note this call will not be handled in the creator thread of the context
 */
HRESULT libtf_get_full_screen(libtf_pInputContext, bool*);

/**
 * @brief Get if input method should show its Candidate Window of the context
 *
 * @note this call will not be handled in the creator thread of the context
 */
HRESULT libtf_get_show_candidate_list_wnd(libtf_pInputContext, bool*);
#pragma endregion

/**
 * @note these call will not be handled in the creator thread of the context
 */
#pragma region setCallback
/**
 * @brief Set Composition Callback of the context
 */
HRESULT libtf_set_composition_callback(libtf_pInputContext, libtf_CallbackComposition);

/**
 * @brief Set Commit Callback of the context
 */
HRESULT libtf_set_commit_callback(libtf_pInputContext, libtf_CallbackCommit);

/**
 * @brief Set PreEdit Bounding Box Callback of the context
 */
HRESULT libtf_set_bounding_box_callback(libtf_pInputContext, libtf_CallbackBoundingBox);

/**
 * @brief Set Candidate List Callback of the context
 */
HRESULT libtf_set_candidate_list_callback(libtf_pInputContext, libtf_CallbackCandidateList);

/**
 * @brief Set Conversion mode Callback of the context
 */
HRESULT libtf_set_conversion_mode_callback(libtf_pInputContext, libtf_CallbackConversionMode);

/**
 * @brief Set Sentence mode Callback of the context
 */
HRESULT libtf_set_sentence_mode_callback(libtf_pInputContext, libtf_CallbackSentenceMode);

/**
 * @brief Set Input Processor Callback of the context
 */
HRESULT libtf_set_input_processor_callback(libtf_pInputContext, libtf_CallbackInputProcessor);
#pragma endregion
}