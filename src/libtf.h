#pragma once
#include "InputContext.hpp"
#include "libtfdef.h"

namespace libtf
{
    typedef struct tagInputContext
    {
        /**
     * @brief Current Window of the context
     */
        HWND hWnd;
        CComPtr<libtf::CInputContext> ctx;
    } InputContext_t;
}

extern "C"
{
    typedef struct libtf::tagInputContext *libtf_pInputContext;
    typedef struct TF_INPUTPROCESSORPROFILE libtf_InputProcessorProfile_t, *libtf_pInputProcessorProfile;
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
    HRESULT libtf_get_input_processors(libtf_InputProcessorProfile_t *profiles, size_t maxSize, size_t *fetched);

    /**
     * @brief Get active input processor profie of current thread
     */
    HRESULT libtf_get_active_input_processor(libtf_InputProcessorProfile_t *);

    /**
     * @brief Set active input processor profie of current thread
     */
    HRESULT libtf_set_active_input_processor(libtf_InputProcessorProfile_t);
#pragma endregion

#pragma region Context
    /**
     * @brief Create input context on calling thread
     */
    HRESULT libtf_create_ctx(libtf_pInputContext *);

    /**
     * @brief Dispose input context
     */
    HRESULT libtf_dispose_ctx(libtf_pInputContext);

    /**
     * @brief Terminate all the compositions in the context
     */
    HRESULT libtf_terminate_composition(libtf_pInputContext);

    /**
     * @brief Set input method state
     */
    HRESULT libtf_set_im_state(libtf_pInputContext, bool);

    /**
     * @brief Get input method state
     * 
     * @return true IM has enabled
     * @return false IM has disabled
     */
    HRESULT libtf_get_im_state(libtf_pInputContext, bool *);

    /**
     * @brief Set current focused window
     * 
     * @param hWnd window who receive WM_SETFOCUS on its message queue
     *             this parameter can be NULL if the context does not have the corresponding handle to the window.
     */
    HRESULT libtf_set_focus_wnd(libtf_pInputContext, HWND);

    /**
     * @brief Get current focused window
     */
    HRESULT libtf_get_focus_wnd(libtf_pInputContext, HWND *);

    /**
     * @brief Set Conversion Mode
     * 
     * @return HRESULT 
     */
    HRESULT libtf_set_conversion_mode(libtf_pInputContext, libtf_ConversionMode);

    /**
     * @brief Set Sentence Mode
     * 
     * @return HRESULT 
     */
    HRESULT libtf_set_sentence_mode(libtf_pInputContext, libtf_SentenceMode);

    /**
     * @brief Set if in Full Screen mode
     * 
     * @return HRESULT 
     */
    HRESULT libtf_set_full_screen(libtf_pInputContext, bool);

    /**
     * @brief Set if input method should show its Candidate Window
     * 
     * @return HRESULT 
     */
    HRESULT libtf_set_show_candidate_list_wnd(libtf_pInputContext, bool);
#pragma endregion

#pragma region setCallback
    /**
     * @brief Set Composition Callback
     */
    HRESULT libtf_set_composition_callback(libtf_pInputContext, libtf_CallbackComposition);

    /**
     * @brief Set Commit Callback
     */
    HRESULT libtf_set_commit_callback(libtf_pInputContext, libtf_CallbackCommit);

    /**
     * @brief Set PreEdit Bounding Box Callback
     */
    HRESULT libtf_set_bounding_box_callback(libtf_pInputContext, libtf_CallbackBoundingBox);

    /**
     * @brief Set Candidate List Callback
     */
    HRESULT libtf_set_candidate_list_callback(libtf_pInputContext, libtf_CallbackCandidateList);

    /**
     * @brief Set Conversion mode Callback
     */
    HRESULT libtf_set_conversion_mode_callback(libtf_pInputContext, libtf_CallbackConversionMode);

    /**
     * @brief Set Sentence mode Callback
     */
    HRESULT libtf_set_sentence_mode_callback(libtf_pInputContext, libtf_CallbackSentenceMode);

    /**
     * @brief Set Input Processor Callback
     */
    HRESULT libtf_set_input_processor_callback(libtf_pInputContext, libtf_CallbackInputProcessor);
#pragma endregion
}