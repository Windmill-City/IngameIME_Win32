#pragma once

#include <functional>
#include <list>
#include <memory>

#include <atlbase.h>
#include <atlcom.h>

#include <msctf.h>

#include "libtfdef.h"

#include "ICallback.hpp"

namespace libtf {
    class CandidateListContext {
        CComQIPtr<ITfCandidateListUIElementBehavior> m_Behavior;
        uint32_t                                     m_PageStart;

      public:
        /**
         * @brief Which candidate string has been selected
         */
        uint32_t m_Selection;
        /**
         * @brief Size of m_Candidates
         */
        uint32_t                m_PageSize;
        std::list<std::wstring> m_Candidates;

      public:
        HRESULT ctor(CComPtr<ITfCandidateListUIElement> ele)
        {
            BEGIN_HRESULT();

            m_Behavior = ele;

            // Total count of Candidates
            uint32_t totalCount;
            CHECK_HR(ele->GetCount(&totalCount));

            // How many pages?
            uint32_t pageCount;
            CHECK_HR(ele->GetPageIndex(NULL, 0, &pageCount));

            // Array of pages' start index
            auto pageStarts = std::make_unique<uint32_t[]>(pageCount);
            CHECK_HR(ele->GetPageIndex(pageStarts.get(), pageCount, &pageCount));

            // Current page's index in pageStarts
            uint32_t curPage;
            CHECK_HR(ele->GetCurrentPage(&curPage));

            m_PageStart      = pageStarts[curPage];
            uint32_t pageEnd = curPage == pageCount - 1 ? totalCount : pageStarts[curPage + 1];
            m_PageSize       = pageEnd - m_PageStart;

            // Currently Selected Candidate's absolute index
            CHECK_HR(ele->GetSelection(&m_Selection));
            // Absolute index to relative index
            m_Selection -= m_PageStart;

            // Get Candidate Strings
            for (uint32_t i = m_PageStart; i < pageEnd; i++) {
                BEGIN_HRESULT_SCOPE();
                CComBSTR candidate;
                CHECK_HR(ele->GetString(i, &candidate));
                m_Candidates.push_back(candidate.m_str);
                goto SkipErr;
                END_HRESULT_SCOPE();
                // If error occurs just put empty string in it
                m_Candidates.push_back(L"");
            SkipErr:;
            }

            END_HRESULT();
        }

      public:
        /**
         * @brief Set currently selected Candidate
         * When user press Space key, input method will commit with currently selected Candidate String
         *
         * @param index Candidate's index
         * @return HRESULT
         */
        HRESULT select(uint32_t index) const
        {
            // Relative index to absolute index
            index += m_PageStart;
            return m_Behavior->SetSelection(index);
        }

        /**
         * @brief Force input method to commit with currently selected Candidate String
         *
         * @return HRESULT
         */
        HRESULT finalize() const
        {
            return m_Behavior->Finalize();
        }
    };

    /**
     * @brief Handle events while input method's candidate list window is hidden
     */
    class CandidateListHandler
        : public ICallback<const libtf_CandidateListState, std::shared_ptr<const CandidateListContext>> {
        CComPtr<ITfCandidateListUIElement> m_UIElement;

      public:
        std::shared_ptr<const CandidateListContext> m_Context;

      public:
        HRESULT BeginUIElement(CComPtr<ITfCandidateListUIElement> ele)
        {
            m_UIElement = ele;

            runCallback(libtf_CandidateListBegin, NULL);

            return S_OK;
        }

        HRESULT UpdateUIElement()
        {
            BEGIN_HRESULT();

            // No active candidatelist
            if (!m_UIElement) return S_OK;

            auto ctx = std::make_shared<CandidateListContext>();
            CHECK_HR(ctx->ctor(m_UIElement));
            m_Context = ctx;

            runCallback(libtf_CandidateListUpdate, std::move(ctx));

            END_HRESULT();
        }

        HRESULT EndUIElement()
        {
            // No active candidatelist
            if (!m_UIElement) return S_OK;

            runCallback(libtf_CandidateListEnd, NULL);

            m_Context.reset();

            return S_OK;
        }
    };
}// namespace libtf