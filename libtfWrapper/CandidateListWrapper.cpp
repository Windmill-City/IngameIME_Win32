#include "pch.h"
#include "CandidateListWrapper.h"

CandidateListWrapper::CandidateListWrapper(UIElementSink* sink, Common* common)
{
	handler = new CandidateListHandler(sink, common);

	sink_candidateList = gcnew CandidateSink_native(this, &CandidateListWrapper::onCandidateList);
	handler->m_sigCandidateList.connect(reinterpret_cast<nativeType>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(sink_candidateList).ToPointer()));
}

#include <msclr\marshal_cppstd.h>
using namespace msclr::interop;
using namespace System;
using namespace System::Collections::Generic;
VOID CandidateListWrapper::onCandidateList(CandidateList* list)
{
	auto reflist = gcnew refCandidateList();
	reflist->Count = list->Count;
	reflist->CurPage = list->CurPage;
	reflist->CurSel = list->CurSel;
	reflist->PageSize = list->PageSize;
	reflist->Candidates = gcnew List<String^>();
	for (size_t i = 0; i < list->PageSize; i++)
	{
		reflist->Candidates->Add(marshal_as<String^>(list->Candidates[i]));
	}
	eventCandidateList(reflist);
}