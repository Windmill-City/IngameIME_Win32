#pragma once
#include <stdint.h>
#include <winerror.h>
#include <wtypes.h>

#define LIBTF_EXPORT __declspec(dllexport)

#define CHECK_HR(hresultItem)                                                                                          \
    {                                                                                                                  \
        HRESULT _hr = hresultItem;                                                                                     \
        if (FAILED(_hr)) return _hr;                                                                                   \
    }

/**
 * @brief Check if ERROR_SUCCESS
 */
#define NOT_ES(hresultItem) (ERROR_SUCCESS != (hresultItem))

/**
 * @brief Check if ERROR_SUCCESS
 */
#define CHECK_ES(hresultItem)                                                                                          \
    {                                                                                                                  \
        HRESULT _hr = hresultItem;                                                                                     \
        if (NOT_ES(_hr)) return _hr;                                                                                   \
    }

#define CHECK_OOM(ptr)                                                                                                 \
    if (!(ptr)) return E_OUTOFMEMORY;

#ifdef __cplusplus
extern "C" {
#else
#    define bool _Bool
#endif
#pragma region CandidateList
LIBTF_EXPORT typedef enum libtf_CandidateListState {
    libtf_CandidateListBegin,
    libtf_CandidateListUpdate,
    libtf_CandidateListEnd
} libtf_CandidateListState_t;
/**
 * @brief Candidate String format is Binary String
 */
LIBTF_EXPORT typedef BSTR libtf_Candidate;
LIBTF_EXPORT typedef struct libtf_tagCandidateList
{
    /**
     * @brief The property below only available at CandidateListUpdate
     */
    libtf_CandidateListState_t state;
    /**
     * @brief Total count of the Candidates
     */
    uint32_t totalCount;
    /**
     * @brief At which index is current page start(inclusive)
     */
    uint32_t pageStart;
    /**
     * @brief At which index is current page end(inclusive)
     */
    uint32_t pageEnd;
    /**
     * @brief At which index is current selected Candidate
     */
    uint32_t curSelection;
    /**
     * @brief Array of Candidates
     *
     * @note The memory of the Candidate will be free after return
     */
    libtf_Candidate* candidates;
} libtf_CandidateList_t, *libtf_pCandidateList;
LIBTF_EXPORT typedef void (*libtf_CallbackCandidateList)(libtf_CandidateList_t, void* userData);
#pragma endregion

#pragma region Commit
/**
 * @note The memory of the Commit will be free after return
 */
LIBTF_EXPORT typedef BSTR libtf_Commit;
LIBTF_EXPORT typedef void (*libtf_CallbackCommit)(libtf_Commit, void* userData);
#pragma endregion

#pragma region Composition
LIBTF_EXPORT typedef enum libtf_CompositionState {
    libtf_CompositionBegin,
    libtf_CompositionUpdate,
    libtf_CompositionEnd
} libtf_CompositionState_t;
LIBTF_EXPORT typedef RECT libtf_BoundingBox_t;
/**
 * @note The memory of the PreEdit will be free after return
 */
LIBTF_EXPORT typedef BSTR libtf_PreEdit;
LIBTF_EXPORT typedef struct libtf_tagComposition
{
    libtf_CompositionState_t state;
    /**
     * @brief Only Available at CompositionUpdate
     */
    libtf_PreEdit preEdit;
    /**
     * @brief Only Available at CompositionUpdate
     */
    long selection[2];
} libtf_Composition_t, *libtf_pComposition;
LIBTF_EXPORT typedef void (*libtf_CallbackComposition)(libtf_Composition_t, void* userData);
LIBTF_EXPORT typedef void (*libtf_CallbackBoundingBox)(libtf_BoundingBox_t*, void* userData);
#pragma endregion

#pragma region                     ConversionMode
LIBTF_EXPORT typedef unsigned long libtf_ConversionMode;
LIBTF_EXPORT typedef void (*libtf_CallbackConversionMode)(libtf_ConversionMode, void* userData);
#pragma endregion

#pragma region                     SentenceMode
LIBTF_EXPORT typedef unsigned long libtf_SentenceMode;
LIBTF_EXPORT typedef void (*libtf_CallbackSentenceMode)(libtf_SentenceMode, void* userData);
#pragma endregion

#pragma region InputProcessor
LIBTF_EXPORT typedef struct tagInputProcessorProfile
{
    /**
     * @brief The type of this profile. This is one of these values.
     * TF_PROFILETYPE_INPUTPROCESSOR - This is a text service.
     * TF_PROFILETYPE_KEYBOARDLAYOUT - This is a keyboard layout.
     */
    DWORD profileType;
    /**
     * @brief Specifies the language id of the profile.
     */
    LANGID langId;
    /**
     * @brief Specifies the CLSID of the text service.
     * If dwProfileType is TF_PROFILETYPE_KEYBOARDLAYOUT, this is CLSID_NULL.
     */
    CLSID clsid;
    /**
     * @brief Specifies the category of this text service.
     * This category is GUID_TFCAT_TIP_KEYBOARD, GUID_TFCAT_TIP_SPEECH, GUID_TFCAT_TIP_HANDWRITING or something in
     * GUID_TFCAT_CATEGORY_OF_TIP. If dwProfileType is TF_PROFILETYPE_KEYBOARDLAYOUT, this is GUID_NULL.
     */
    GUID catid;
    /**
     * @brief Specifies the GUID to identify the profile.
     * If dwProfileType is TF_PROFILETYPE_KEYBOARDLAYOUT, this is GUID_NULL.
     */
    GUID guidProfile;
    /**
     * @brief Specifies the keyboard layout handle of this profile.
     * If dwProfileType is TF_PROFILETYPE_ INPUTPROCESSOR, this is NULL.
     */
    HKL hkl;
    /**
     * @brief If the InputProcessor activated
     */
    bool activated;
} libtf_InputProcessorProfile_t, *libtf_pInputProcessorProfile;
LIBTF_EXPORT typedef void (*libtf_CallbackInputProcessor)(libtf_InputProcessorProfile_t, void* userData);
#pragma endregion
#ifdef __cplusplus
}
#endif