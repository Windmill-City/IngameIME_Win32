#pragma once
#include "UIElementSink.h"
#include <boost/bind.hpp>

namespace libtf {
	struct TFAPI CandidateList
	{
	public:
		LONG			Count;
		LONG			CurSel;
		LONG			PageSize;
		LONG			CurPage;
		std::wstring* Candidates;//PageSize indicates its array length

		CandidateList() {
			Reset();
		}

		VOID Reset()
		{
			Count = 0;
			CurSel = 0;
			PageSize = 0;
			CurPage = 0;
			Candidates = new std::wstring[0];
		}
	};

	class TFAPI CandidateListHandler
	{
	private:
		Common* m_common;
		typedef boost::signals2::signal<VOID(CandidateList* list)> signal_CandidateList;
	public:
		CandidateList* m_list;
		signal_CandidateList m_sigCandidateList;

		CandidateListHandler(UIElementSink* sink, Common* common) {
			m_common = common;
			m_list = new CandidateList();

			sink->m_sigUIElement.connect(boost::bind(&CandidateListHandler::onUIEle, this, _1));
		}

		VOID onUIEle(UIElementEventArgs* args) {
			switch (args->m_state)
			{
			case UIElementState::Begin:
				args->m_pfShow = FALSE;
			case UIElementState::End:
				m_list->Reset();
				break;
			case UIElementState::Update:
				fetchCandidateList(args);
				break;
			default:
				break;
			}
			m_sigCandidateList(m_list);
		}

		VOID fetchCandidateList(UIElementEventArgs* args) {
			ITfUIElementMgr* UIMgr = m_common->m_pUIElementMgr;
			CComPtr<ITfUIElement> uiElement;
			if (SUCCEEDED(UIMgr->GetUIElement(args->m_dwUIElementId, &uiElement)))
			{
				CComPtr<ITfCandidateListUIElement> candidateListUIEle;
				if (SUCCEEDED(uiElement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidateListUIEle)))
				{
					UINT count;
					candidateListUIEle->GetCount(&count);
					UINT pcount;
					candidateListUIEle->GetPageIndex(NULL, 0, &pcount);
					UINT* pages = new UINT[pcount];
					candidateListUIEle->GetPageIndex(pages, pcount, &pcount);
					UINT cpage;
					candidateListUIEle->GetCurrentPage(&cpage);
					UINT csel;
					candidateListUIEle->GetSelection(&csel);
					m_list->CurSel = csel;
					m_list->Count = count;
					m_list->CurPage = cpage;

					UINT pageSize = cpage == pcount - 1 ? count % pcount : count / pcount;
					m_list->PageSize = pageSize;
					UINT start = pages[cpage];
					UINT end = start + pageSize;

					m_list->Candidates = new std::wstring[pageSize];
					int j = 0;
					for (UINT i = start; i < end; i++, j++)
					{
						CComBSTR candidate;
						if (SUCCEEDED(candidateListUIEle->GetString(i, &candidate)))
						{
							LPWSTR text = candidate;

							m_list->Candidates[j] = text;
						}
					}
				}
			}
		}
	};
}
