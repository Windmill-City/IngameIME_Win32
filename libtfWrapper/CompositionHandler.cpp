#include "pch.h"
#include "CompositionHandler.h"

CompositionHandler::CompositionHandler(TextStore* ts)
{
	sink_comp = gcnew CompositionSink_native(this, &CompositionHandler::onComposition);
	ts->m_sigComposition.connect(reinterpret_cast<CompositionSink_nativeType>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(sink_comp).ToPointer()));

	sink_ext = gcnew CompositionExtSink_native(this, &CompositionHandler::onCompositionExt);
	ts->m_sigGetCompExt.connect(reinterpret_cast<CompositionExtSink_nativeType>(System::Runtime::InteropServices::Marshal::GetFunctionPointerForDelegate(sink_ext).ToPointer()));
}
#include <msclr\marshal_cppstd.h>
using namespace msclr::interop;
using namespace System;
VOID CompositionHandler::onComposition(ITfContextOwnerCompositionSink* sink, CompositionEventArgs* comp)
{
	auto refComp = gcnew refCompositionEventArgs();
	refComp->m_state = (refCompositionState)comp->m_state;
	refComp->m_caretPos = comp->m_caretPos;
	refComp->m_strComp = marshal_as<String^>(comp->m_strComp);
	refComp->m_strCommit = marshal_as<String^>(comp->m_strCommit);
	eventComposition(refComp);
}

VOID CompositionHandler::onCompositionExt(TextStore* textStore, RECT* rect)
{
	eventGetCompExt((System::IntPtr)rect);
}