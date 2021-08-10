#pragma once
#include "InputContext.hpp"
#include "TfThread.hpp"
#include "libtfdef.h"

typedef struct InputContext
{
    /**
     * @brief Current Window of the context
     */
    HWND hWnd;
    /**
     * @brief STA Apartment Thread
     */
    libtf::TfThread *tfThread;
    CComPtr<libtf::CInputContext> ctx;
} InputContext_t, *pInputContext;

extern "C"
{
    /**
     * @brief Create input context on calling thread
     */
    HRESULT create(pInputContext);

    /**
     * @brief Dispose input context
     */
    HRESULT dispose(pInputContext);

    /**
     * @brief Terminate all the compositions in the context
     */
    HRESULT terminateComposition(pInputContext);

    /**
     * @brief Set input method state
     */
    HRESULT setIMState(pInputContext, bool);

    /**
     * @brief Get input method state
     * 
     * @return true IM has enabled
     * @return false IM has disabled
     */
    HRESULT getIMState(pInputContext, bool *);

    /**
     * @brief Set current focused window
     * 
     * @param hWnd window who receive WM_SETFOCUS on its message queue
     *             this parameter can be NULL if the context does not have the corresponding handle to the window.
     */
    HRESULT setFocus(pInputContext, HWND);

    /**
     * @brief Get current focused window
     */
    HRESULT getFocus(pInputContext, HWND *);

    /**
     * @brief Set Conversion Mode
     * 
     * @return HRESULT 
     */
    HRESULT setConversionMode(pInputContext, ConversionMode);

    /**
     * @brief Set Sentence Mode
     * 
     * @return HRESULT 
     */
    HRESULT setSentenceMode(pInputContext, SentenceMode);

    /**
     * @brief Set if in Full Screen mode
     * 
     * @return HRESULT 
     */
    HRESULT setFullScreen(pInputContext, bool);

    /**
     * @brief Set if input method should show its Candidate Window
     * 
     * @return HRESULT 
     */
    HRESULT setShowCandidateWnd(pInputContext, bool);

#pragma region setCallback
    /**
     * @brief Set Composition Callback
     */
    HRESULT setCompositionCallback(pInputContext, CallbackComposition);

    /**
     * @brief Set PreEdit Bounding Box Callback
     */
    HRESULT setBoundingBoxCallback(pInputContext, CallbackBoundingBox);

    /**
     * @brief Set Candidate List Callback
     */
    HRESULT setCandidateListCallback(pInputContext, CallbackCandidateList);

    /**
     * @brief Set Conversion mode Callback
     */
    HRESULT setConversionModeCallback(pInputContext, CallbackConversionMode);

    /**
     * @brief Set Sentence mode Callback
     */
    HRESULT setSentenceModeCallback(pInputContext, CallbackSentenceMode);
#pragma endregion
}