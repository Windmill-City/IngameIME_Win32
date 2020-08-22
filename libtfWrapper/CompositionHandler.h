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

	CompositionHandler(TextStore ts);
	VOID onComposition(ContextOwnerCompositionSink* sink, CompositionEventArgs* comp);
	VOID onCompositionExt(TextStore* textStore, RECT* rect);
};
