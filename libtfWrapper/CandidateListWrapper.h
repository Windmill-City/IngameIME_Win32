#pragma once
#include "../libtf/CandidateListHandler.h"
#include "../libtf/tf_application.h"
using namespace libtf;
public ref class CandidateListWrapper
{
private:
	typedef VOID(*nativeType)(CandidateList* list);
	delegate VOID CandidateSink_native(CandidateList* list);

	CandidateListHandler* handler;
	CandidateSink_native^ sink_candidateList;
public:
	delegate VOID CandidateSink_cli(System::IntPtr list);
	event CandidateSink_cli^ eventCandidateList;

	CandidateListWrapper(UIElementSink* sink, Common* common);
	VOID onCandidateList(CandidateList* list);
};
