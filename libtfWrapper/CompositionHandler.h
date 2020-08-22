#pragma once
#include "../libtf/TextStore.h"
#include "Delegate.h"

using namespace libtf;
public ref class CompositionHandler
{
#pragma region EventHandler Def&Var
	//CPP
	typedef Delegate<VOID(ContextOwnerCompositionSink* sink, CompositionEventArgs* comp),
		VOID(System::IntPtr comp)> CompositionSink;

	typedef Delegate<VOID(TextStore* textStore, RECT* rect),
		VOID(System::IntPtr rect)> CompositionExtSink;

	CompositionSink::PCLI_CALL sink_comp;
	CompositionExtSink::PCLI_CALL sink_ext;
#pragma endregion
public:
	CompositionSink::EVENT eventComposition;
	CompositionExtSink::EVENT eventGetCompExt;

	CompositionHandler(TextStore ts) {
		sink_comp = gcnew CompositionSink::CLI_CALL(this, &CompositionHandler::onComposition);
		ts.m_sigComposition.connect(CompositionSink::GetPointerForNative(sink_comp));

		sink_ext = gcnew CompositionExtSink::CLI_CALL(this, &CompositionHandler::onCompositionExt);
		ts.m_sigGetCompExt.connect(CompositionExtSink::GetPointerForNative(sink_ext));
	}

	VOID onComposition(ContextOwnerCompositionSink* sink, CompositionEventArgs* comp) {
		eventComposition((System::IntPtr)comp);
	}

	VOID onCompositionExt(TextStore* textStore, RECT* rect) {
		eventGetCompExt((System::IntPtr)rect);
	}
};
