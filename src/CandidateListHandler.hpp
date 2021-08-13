#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <memory>
#include <msctf.h>

extern "C" {
typedef enum libtf_CandidateListState {
    libtf_CandidateListBegin,
    libtf_CandidateListUpdate,
    libtf_CandidateListEnd
} libtf_CandidateListState_t;
/**
 * @brief Candidate String format is Binary String
 */
typedef BSTR libtf_Candidate;
typedef struct libtf_tagCandidateList
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
     */
    libtf_Candidate* candidates;
} libtf_CandidateList_t, *libtf_pCandidateList;
typedef void (*libtf_CallbackCandidateList)(libtf_CandidateList_t);
}

namespace libtf {
    /**
     * @brief Default way to get CandidateList from ITfCandidateListUIElement
     */
    typedef class CandidateListProvider {
        typedef CComPtr<ITfCandidateListUIElement> CandUIEle;

      public:
        /**
         * @brief Get the total count of the candidate list
         */
        virtual HRESULT getTotalCount(CandUIEle candEle, uint32_t& totalCount)
        {
            return candEle->GetCount(&totalCount);
        }

        /**
         * @brief Get the Page Start and End
         */
        virtual HRESULT getPageStartAndEnd(CandUIEle candEle, uint32_t& start, uint32_t& end)
        {
            uint32_t totalCount;
            CHECK_HR(getTotalCount(candEle, totalCount));

            uint32_t pageCount;
            CHECK_HR(candEle->GetPageIndex(NULL, 0, &pageCount));

            uint32_t* pageStarts = new uint32_t[pageCount];
            CHECK_OOM(pageStarts);
            CHECK_HR(candEle->GetPageIndex(pageStarts, pageCount, &pageCount));

            uint32_t curPage;
            CHECK_HR(candEle->GetCurrentPage(&curPage));

            bool isLastPage = curPage == pageCount - 1;
            start           = pageStarts[curPage];
            end             = isLastPage ? totalCount - 1 : pageStarts[curPage + 1] - 1;
            delete[] pageStarts;

            return S_OK;
        }

        /**
         * @brief Get current selected Candidate's index
         */
        virtual HRESULT getCurSelection(CandUIEle candEle, uint32_t& index)
        {
            return candEle->GetSelection(&index);
        }

        /**
         * @brief Get Candidate by its index
         */
        virtual HRESULT getCandidate(CandUIEle candEle, size_t index, libtf_Candidate* cand)
        {
            CHECK_HR(candEle->GetString((UINT)index, cand));
            return S_OK;
        }

        virtual HRESULT getCandidateList(CandUIEle candEle, libtf_pCandidateList list)
        {
            CHECK_HR(getTotalCount(candEle, list->totalCount));
            CHECK_HR(getPageStartAndEnd(candEle, list->pageStart, list->pageEnd));
            CHECK_HR(getCurSelection(candEle, list->curSelection));

            list->candidates = new libtf_Candidate[list->totalCount];
            CHECK_OOM(list->candidates);

            HRESULT hr;
            for (size_t i = 0; i < list->totalCount; i++) {
                if (FAILED(hr = getCandidate(candEle, i, &list->candidates[i]))) {
                    // Cleanup
                    for (size_t k = 0; k < i; k++) { SysFreeString(list->candidates[k]); }
                    delete[] list->candidates;
                    return hr;
                }
            }
            return S_OK;
        }
    } CandidateListProvider_t, *pCandidateListProvider;

    /**
     * @brief Get data from input method's CandidateList UIElement
     */
    class CandidateListHandler : public CComObjectRoot, public ITfUIElementSink {
      protected:
        CComPtr<ITfUIElementMgr>           m_uiElementMgr;
        DWORD                              m_uiSinkCookie = TF_INVALID_COOKIE;
        CComPtr<ITfCandidateListUIElement> m_curCandEle;

        /**
         * @brief Get the Candidate List UI Element object
         *
         * @param dwUIElementId uiElement id
         * @return CComPtr<ITfCandidateListUIElement>
         */
        CComPtr<ITfCandidateListUIElement> getCandidateListUIElement(DWORD dwUIElementId)
        {
            CComPtr<ITfUIElement>                uiElement;
            CComQIPtr<ITfCandidateListUIElement> candidateList;
            if (FAILED(m_uiElementMgr->GetUIElement(dwUIElementId, &uiElement))) { return candidateList; }
            candidateList = uiElement;
            return candidateList;
        }

      public:
        /**
         * @brief Set if input method should(IM) show its candidate list window
         *
         * Set to false to get candidate data from IM
         */
        bool m_show = true;

        /**
         * @brief Callback when Candidate List updates
         */
        typedef std::function<void(libtf_CandidateList_t)> signalCandidateList;
        signalCandidateList                                m_sigCandidateList = [](libtf_CandidateList_t) {};

        /**
         * @brief Specific provider for different input method
         */
        std::unique_ptr<CandidateListProvider> provider = std::make_unique<CandidateListProvider_t>();

        BEGIN_COM_MAP(CandidateListHandler)
        COM_INTERFACE_ENTRY(ITfUIElementSink)
        END_COM_MAP()

        /**
         * @brief Initialize the handler
         *
         * @param uiElementMgr Query interface from ITfThreadMgr
         * @return HRESULT
         */
        HRESULT initialize(CComPtr<ITfUIElementMgr> uiElementMgr)
        {
            m_uiElementMgr           = uiElementMgr;
            CComQIPtr<ITfSource> evt = m_uiElementMgr;
            return evt->AdviseSink(IID_ITfUIElementSink, this, &m_uiSinkCookie);
        }

        /**
         * @brief Dispose the handler
         *
         * @return HRESULT
         */
        HRESULT dispose()
        {
            CComQIPtr<ITfSource> evtSource = m_uiElementMgr;
            return evtSource->UnadviseSink(m_uiSinkCookie);
        }

        /**
         * @brief Set current selected Candidate
         * When user pressing Space key, input method will commit current selected Candidate
         *
         * @param index Candidate index
         * @return HRESULT
         */
        HRESULT setSelection(uint32_t index)
        {
            CComQIPtr<ITfCandidateListUIElementBehavior> candBehavior;
            if (candBehavior = m_curCandEle) {
                CHECK_HR(candBehavior->SetSelection(index));
                return S_OK;
            }
            return E_FAIL;
        }

        /**
         * @brief Finalize with specific Candidate
         * Let input method commit with specific Candidate
         *
         * @param index Candidate index
         * @return HRESULT
         */
        HRESULT setFinalize(uint32_t index)
        {
            CComQIPtr<ITfCandidateListUIElementBehavior> candBehavior;
            if (candBehavior = m_curCandEle) {
                CHECK_HR(candBehavior->SetSelection(index));
                CHECK_HR(candBehavior->Finalize());
                return S_OK;
            }
            return E_FAIL;
        }

        /**
         * @brief Decide if input method should show its candidate window
         *
         * @param pbShow should show?
         */
        HRESULT BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override
        {
            // It's possible that other uiElement Begins while handling Candidate List
            // to avoid wrongly overwrite, we use a local var here
            auto uiElement = getCandidateListUIElement(dwUIElementId);
            if (uiElement) {
                m_curCandEle = uiElement;

                *pbShow = m_show;

                m_sigCandidateList({libtf_CandidateListBegin});
            }
            return S_OK;
        }

        /**
         * @brief Get Candidates from input method
         */
        HRESULT UpdateUIElement(DWORD dwUIElementId) override
        {
            if (m_curCandEle == getCandidateListUIElement(dwUIElementId)) {
                libtf_CandidateList_t list = {libtf_CandidateListUpdate};
                CHECK_HR(provider->getCandidateList(m_curCandEle, &list));

                m_sigCandidateList(list);

                // Cleanup
                for (size_t i = 0; i < list.totalCount; i++) { SysFreeString(list.candidates[i]); }
                delete[] list.candidates;
            }
            return S_OK;
        }

        /**
         * @brief Candidate list End
         */
        HRESULT EndUIElement(DWORD dwUIElementId) override
        {
            if (m_curCandEle == getCandidateListUIElement(dwUIElementId)) {
                // Always Release CandUIEle
                m_curCandEle.Release();

                m_sigCandidateList({libtf_CandidateListEnd});
            }
            return S_OK;
        }
    };

    typedef CComObjectNoLock<CandidateListHandler> CCandidateListHandler;
}// namespace libtf