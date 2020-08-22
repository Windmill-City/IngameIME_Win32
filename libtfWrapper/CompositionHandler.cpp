#include "pch.h"
#include "CompositionHandler.h"

CompositionHandler::CompositionHandler(TextStore ts)
{
	sink_comp = gcnew CompositionSink::CLI_DLG(this, &CompositionHandler::onComposition);
	ts.m_sigComposition.connect(CompositionSink::GetPointerForNative(sink_comp));

	sink_ext = gcnew CompositionExtSink::CLI_DLG(this, &CompositionHandler::onCompositionExt);
	ts.m_sigGetCompExt.connect(CompositionExtSink::GetPointerForNative(sink_ext));
}

VOID CompositionHandler::onComposition(ContextOwnerCompositionSink* sink, CompositionEventArgs* comp)
{
	eventComposition((System::IntPtr)comp);
}

VOID CompositionHandler::onCompositionExt(TextStore* textStore, RECT* rect)
{
	eventGetCompExt((System::IntPtr)rect);
}