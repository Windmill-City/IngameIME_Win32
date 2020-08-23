#include "pch.h"
#include "CandidateListWrapper.h"

CandidateListWrapper::CandidateListWrapper(UIElementSink* sink, Common* common)
{
	handler = new CandidateListHandler(sink, common);

	sink_candidateList = gcnew CandidateSink_native(this, &CandidateListWrapper::onCandidateList);
	handler->m_sigCandidateList.connect(reinterpret_cast<nativeType>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(sink_candidateList).ToPointer()));
}

VOID CandidateListWrapper::onCandidateList(CandidateList* list)
{
	eventCandidateList((System::IntPtr)list);
}