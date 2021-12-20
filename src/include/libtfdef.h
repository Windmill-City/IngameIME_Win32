#pragma once
#include <stdint.h>
#include <winerror.h>
#include <wtypes.h>

#define LIBTF_EXPORT __declspec(dllexport)

#define BEGIN_HRESULT_SCOPE()                                                                                          \
    try {                                                                                                              \
        do {
#define END_HRESULT_SCOPE()                                                                                            \
    }                                                                                                                  \
    while (0)                                                                                                          \
        ;                                                                                                              \
    }                                                                                                                  \
    catch (std::bad_alloc)                                                                                             \
    {                                                                                                                  \
        hr = E_OUTOFMEMORY;                                                                                            \
    }
#define BEGIN_HRESULT_AS(x)                                                                                            \
    HRESULT hr = x;                                                                                                    \
    BEGIN_HRESULT_SCOPE();
#define END_HRESULT()                                                                                                  \
    END_HRESULT_SCOPE();                                                                                               \
    return hr;
#define BEGIN_HRESULT() BEGIN_HRESULT_AS(S_OK)
#define CHECK_HR(hresultItem)                                                                                          \
    if (FAILED(hr = hresultItem)) break;
#define CHECK_ES(hresultItem)                                                                                          \
    if (ERROR_SUCCESS != (hr = hresultItem)) break;

#ifdef __cplusplus
extern "C" {
#else
#    define bool _Bool
#endif
#pragma region                                  InputContext
LIBTF_EXPORT typedef struct libtf_InputContext* libtf_pInputContext;
#pragma endregion
#pragma region CandidateList
LIBTF_EXPORT typedef enum libtf_CandidateListState {
    libtf_CandidateListBegin,
    libtf_CandidateListUpdate,
    libtf_CandidateListEnd
} libtf_CandidateListState_t;

LIBTF_EXPORT typedef struct libtf_CandidateListContext
{
    /**
     * @brief Which candidate string has been selected
     */
    uint32_t m_Selection;
    /**
     * @brief Size of m_Candidates
     */
    uint32_t m_PageSize;
    /**
     * @brief Zero-terminated wchar strings
     */
#pragma warning(push)
#pragma warning(disable : 4200)
    wchar_t* m_Candidates[0];
#pragma warning(pop)
} libtf_CandidateListContext_t, *libtf_pCandidateListContext;
LIBTF_EXPORT typedef void (*libtf_CandidateListCallback)(libtf_CandidateListState_t         state,
                                                         const libtf_CandidateListContext_t ctx,
                                                         void*                              userData);
#pragma endregion
#pragma region Composition
LIBTF_EXPORT typedef enum libtf_CompositionState {
    libtf_CompositionBegin,
    libtf_CompositionUpdate,
    libtf_CompositionEnd
} libtf_CompositionState_t;
LIBTF_EXPORT typedef struct libtf_PreEditContext
{
  public:
    /**
     * @brief Start index of the selection(inclusive)
     */
    uint32_t m_SelStart;
    /**
     * @brief End index of the selection(exclusive)
     */
    uint32_t m_SelEnd;
    /**
     * @brief Zero-terminated string
     */
#pragma warning(push)
#pragma warning(disable : 4200)
    wchar_t m_Content[0];
#pragma warning(pop)
} libtf_PreEditContext_t, *libtf_pPreEditContext;
LIBTF_EXPORT typedef void (*libtf_PreEditCallback)(libtf_CompositionState_t     state,
                                                   const libtf_PreEditContext_t ctx,
                                                   void*                        userData);
/**
 * @brief Return the boundary rectangle of the preedit, in window coordinate
 *
 * @note If the length of preedit is 0 (as it would be drawn by input method), the rectangle
 * coincides with the insertion point, and its width is 0.
 */
LIBTF_EXPORT typedef void (*libtf_PreEditRectCallback)(RECT* rect, void* userData);
/**
 * @brief Receives commit string from input method
 *
 * @param commit Zero-terminated string
 */
LIBTF_EXPORT typedef void (*libtf_CommitCallback)(const wchar_t* commit, void* userData);
#pragma endregion
#pragma region InputProcessor
LIBTF_EXPORT typedef enum libtf_InputProcessorType {
    libtf_KeyboardLayout,
    libtf_TextService
} libtf_InputProcessorType_t;
LIBTF_EXPORT typedef void* libtf_HInputProcessor;
LIBTF_EXPORT typedef struct libtf_InputProcessorProfile
{
    /**
     * @brief Type of the InputProcessor
     */
    libtf_InputProcessorType_t m_Type;
    /**
     * @brief Locale string of the InputProcessor
     *
     * Zero-terminated string
     */
    wchar_t* m_Locale;
    /**
     * @brief Localized name of the locale
     *
     * Zero-terminated string
     */
    wchar_t* m_LocaleName;
    /**
     * @brief Localized name of the InputProcessor
     *
     * Zero-terminated string
     */
    wchar_t* m_InputProcessorName;
} libtf_InputProcessorProfile_t, *libtf_pInputProcessorProfile;
LIBTF_EXPORT typedef enum libtf_InputProcessorState {
    /**
     * @brief When active input processor change
     *
     * @note the InputMode will also change
     */
    libtf_InputProcessorFullUpdate,
    /**
     * @brief When only the input mode has changes
     */
    libtf_InputProcessorInputModeUpdate
} libtf_InputProcessorState_t;
LIBTF_EXPORT typedef struct libtf_InputProcessorContext
{
    /**
     * @brief Handle of the active input processor
     */
    libtf_HInputProcessor m_InputProcessor;
    /**
     * @brief Size of m_InputModes
     */
    uint32_t m_InputModeSize;
    /**
     * @brief InputModes of the InputProcessor
     *
     * InputProcessor have different InputModes at different states
     * eg: KeyboardLayout, which has only one state -> 'AlphaNumeric'
     * eg: TextService, in East-Asia, has the following state:
     * {Native, AlphaNumeric}, {HalfShape, FullShape}
     * TextService, in Japan, has extra state:
     * {Hiragana, Katakana, Roman}
     * In the brace, are mutually exclusive InputModes
     */
#pragma warning(push)
#pragma warning(disable : 4200)
    wchar_t* m_InputModes[0];
#pragma warning(pop)
} libtf_InputProcessorContext_t, *libtf_pInputProcessorContext;
LIBTF_EXPORT typedef void (*libtf_InputProcessorCallback)(libtf_InputProcessorState_t         state,
                                                          const libtf_InputProcessorContext_t ctx,
                                                          void*                               userData);
LIBTF_EXPORT typedef struct libtf_InputProcessors
{
    uint32_t m_InputProcessorsSize;
#pragma warning(push)
#pragma warning(disable : 4200)
    libtf_HInputProcessor m_InputProcessors[0];
#pragma warning(pop)
} libtf_InputProcessors_t, *libtf_pInputProcessors;
#pragma endregion
#ifdef __cplusplus
}
#endif