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

	CandidateListWrapper(UIElementSink* sink, Common* common);
	VOID onCandidateList(CandidateList* list);
};
