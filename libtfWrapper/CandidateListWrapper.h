#pragma once
#include "../libtf/CandidateListHandler.h"
#include "../libtf/tf_application.h"
#include "Delegate.h"
using namespace libtf;
public ref class CandidateListWrapper
{
private:
	typedef Delegate<VOID(CandidateList* list), VOID(System::IntPtr list)> CandidateSink;
	CandidateSink::PCLI_CALL sink_candidateList;

	CandidateListHandler* handler;
public:
	event CandidateSink::EVENT eventCandidateList;

	CandidateListWrapper(UIElementSink* sink, Common* common) {
		handler = new CandidateListHandler(sink, common);

		sink_candidateList = gcnew CandidateSink::CLI_CALL(this, &CandidateListWrapper::onCandidateList);
		sink->m_sigUIElement.connect(CandidateSink::GetPointerForNative(sink_candidateList));
	}

	VOID onCandidateList(CandidateList* list) {
		eventCandidateList((System::IntPtr)list);
	}
};
