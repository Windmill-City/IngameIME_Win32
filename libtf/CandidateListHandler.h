#pragma once
#include <iostream>
#include <functional>
#include "UIElementSink.h"

namespace libtf {
	struct TFAPI CandidateList
	{
	public:
		LONG			Count;
		LONG			CurSel;
		LONG			PageSize;
		LONG			CurPage;
		std::shared_ptr<std::wstring[]> Candidates;//PageSize indicates its array length

		CandidateList() {
			Reset();
		}

		VOID Reset()
		{
			Count = 0;
			CurSel = 0;
			PageSize = 0;
			CurPage = 0;
			Candidates.reset(new std::wstring[0]);
		}
	};

	class TFAPI CandidateListHandler
	{
	private:
		Common* m_common;
		typedef std::function<VOID(CandidateList* list)> signal_CandidateList;
	public:
		CandidateList* m_list;
		signal_CandidateList m_sigCandidateList;

		CandidateListHandler(UIElementSink* sink, Common* common) {
			m_common = common;
			m_list = new CandidateList();

			sink->m_sigUIElement = std::bind(&CandidateListHandler::onUIEle, this, std::placeholders::_1);
		}

		VOID onUIEle(UIElementEventArgs* args) {
			switch (args->m_state)
			{
			case UIElementState::Begin:
				*(args->m_pfShow) = FALSE;
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
					HRESULT hr;
					UINT count;
					hr = candidateListUIEle->GetCount(&count);
					RETURNHRVOID(hr);
					UINT pcount;
					hr = candidateListUIEle->GetPageIndex(NULL, 0, &pcount);
					RETURNHRVOID(hr);
					UINT* pages = new UINT[pcount];
					hr = candidateListUIEle->GetPageIndex(pages, pcount, &pcount);
					RETURNHRVOID(hr);
					UINT cpage;
					hr = candidateListUIEle->GetCurrentPage(&cpage);
					RETURNHRVOID(hr);
					UINT csel;
					hr = candidateListUIEle->GetSelection(&csel);
					RETURNHRVOID(hr);
					if (csel == 0xffffffff) return;//For MS JAP
					m_list->CurSel = csel;
					m_list->Count = count;
					m_list->CurPage = cpage;

					UINT pageSize;
					if (count >= pcount)//For MS PinYin and Chinese IME
						pageSize = cpage == pcount - 1 && pcount != 1 ? count % pcount : count / pcount;
					else
						pageSize = pcount > 10 ? 10 : pcount;//For MS JAP
					UINT start = pages[cpage];
					UINT end = start + pageSize;

					m_list->Candidates.reset(new std::wstring[pageSize]);
					int j = 0;
					for (UINT i = start; i < end; i++, j++)
					{
						CComBSTR candidate;
						if (SUCCEEDED(candidateListUIEle->GetString(i, &candidate)))
						{
							LPWSTR text = candidate;
							if (!text) {//some IME may return null str
								j--;
								continue;
							}
							m_list->Candidates[j] = text;
						}
					}
					m_list->PageSize = j;
				}
			}
		}
	};
}
