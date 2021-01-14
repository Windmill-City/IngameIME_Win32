#pragma once
#include <iostream>
#include <functional>
#include "UIElementSink.hpp"
namespace libtf {
	struct TFAPI CandidateList
	{
	public:
		LONG								m_lPageSize;
		std::shared_ptr<std::wstring[]>		m_pCandidates;//PageSize indicates its array length

		CandidateList() {
			Reset();
		}

		VOID Reset()
		{
			m_lPageSize = 0;;
			m_pCandidates.reset();
		}
	};

	class TFAPI CandidateListHandler
	{
		typedef std::function<VOID(CandidateList* list)>	signal_CandidateList;
		CComPtr<ITfUIElementMgr>							m_pUIElementMgr;
		std::shared_ptr<UIElementSink>						m_pUIElementSink;
		std::unique_ptr<CandidateList>						m_pCandidateList;
	public:
		signal_CandidateList								m_sigCandidateList = [](CandidateList* list) {};
		BOOL												m_fhandleCandidate = FALSE;

		CandidateListHandler(CComPtr<ITfUIElementMgr> uiElementMgr, std::shared_ptr<UIElementSink> sink) {
			m_pCandidateList.reset(new CandidateList());
			m_pUIElementSink = sink;
			m_pUIElementMgr = uiElementMgr;
			sink->m_sigUIElement = std::bind(&CandidateListHandler::onUIEle, this, std::placeholders::_1);
		}

		~CandidateListHandler() {
			m_pUIElementSink->m_sigUIElement = [](UIElementEventArgs*) {};
		}

		VOID onUIEle(UIElementEventArgs* args) {
			if (!m_fhandleCandidate) return;
			switch (args->m_state)
			{
			case UIElementState::Begin:
				*(args->m_pfShow) = FALSE;
			case UIElementState::End:
				m_pCandidateList->Reset();
				break;
			case UIElementState::Update:
				fetchCandidateList(args);
				break;
			default:
				break;
			}
			m_sigCandidateList(m_pCandidateList.get());
		}

		VOID fetchCandidateList(UIElementEventArgs* args) {
			CComPtr<ITfUIElement> uiElement;
			if (SUCCEEDED(m_pUIElementMgr->GetUIElement(args->m_dwUIElementId, &uiElement)))
			{
				CComPtr<ITfCandidateListUIElement> candidateListUIEle;
				if (SUCCEEDED(uiElement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidateListUIEle)))
				{
					UINT total;//Total candidates
					UINT pageCount;//Pages' count
					UINT cpage;
					UINT csel;//Current selected candidate
					RET_VOID(candidateListUIEle->GetCount(&total));

					RET_VOID(candidateListUIEle->GetPageIndex(NULL, 0, &pageCount));
					UINT* startIndexs = new UINT[pageCount];//every page's first candidate's index, get by ITfCandidateListUIElement->GetString(Index, &string)
					RET_VOID(candidateListUIEle->GetPageIndex(startIndexs, pageCount, &pageCount));

					RET_VOID(candidateListUIEle->GetCurrentPage(&cpage));
					RET_VOID(candidateListUIEle->GetSelection(&csel));

					if (csel == 0xffffffff) return;//when Microsoft Japanese IME has no avaliable candidates

					UINT pageSize;
					if (total >= pageCount)//Mircosoft Japanese IME's total is 0, while its pageCount is total, maybe a bug?
						pageSize = cpage == pageCount - 1//page index start from 0
						&& pageCount != 1//only one page, take total as page size
						? total % pageCount : total / pageCount;//devide total count by page count to calculate the page size
					else
						pageSize = pageCount > 10 ? 10 : pageCount;

					UINT start = startIndexs[cpage];
					UINT end = start + pageSize;

					m_pCandidateList->m_pCandidates.reset(new std::wstring[pageSize]);
					int j = 0;
					for (UINT i = start; i < end; i++, j++)
					{
						CComBSTR candidate;
						if (SUCCEEDED(candidateListUIEle->GetString(i, &candidate)))
						{
							LPWSTR text = candidate;
							if (!text) {//Mircosoft Japanese IME may return null string
								j--;
								continue;
							}
							m_pCandidateList->m_pCandidates[j] = text;
						}
					}
					m_pCandidateList->m_lPageSize = j;
				}
			}
		}
	};
}
