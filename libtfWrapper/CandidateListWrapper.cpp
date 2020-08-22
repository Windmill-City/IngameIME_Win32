#include "pch.h"
#include "CandidateListWrapper.h"

CandidateListWrapper::CandidateListWrapper(UIElementSink* sink, Common* common)
{
	handler = new CandidateListHandler(sink, common);

	sink_candidateList = gcnew CandidateSink::CLI_DLG(this, &CandidateListWrapper::onCandidateList);
	sink->m_sigUIElement.connect(CandidateSink::GetPointerForNative(sink_candidateList));
}

VOID CandidateListWrapper::onCandidateList(CandidateList* list)
{
	eventCandidateList((System::IntPtr)list);
}