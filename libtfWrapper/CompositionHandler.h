#pragma once
#include "../libtf/TextStore.h"

public enum class refCompositionState
{
	StartComposition,
	Composing,
	Commit,
	EndComposition
};

public ref struct refCompositionEventArgs
{
public:
	refCompositionState				m_state;
	LONG                            m_caretPos;
	System::String^ m_strComp;
	System::String^ m_strCommit;
};

using namespace libtf;
//Wraning, due to some unknown error, this handler cant remove its connect to the signal, dont release this if any composition will make
//it is rarely to release this handler in the whole app lifetime, so just ignore this problem
public ref class CompositionHandler
{
#pragma region EventHandler Def&Var
	//CPP
	typedef VOID(*CompositionSink_nativeType)(ITfContextOwnerCompositionSink* sink, CompositionEventArgs* comp);
	delegate VOID CompositionSink_native(ITfContextOwnerCompositionSink* sink, CompositionEventArgs* comp);

	typedef VOID(*CompositionExtSink_nativeType)(TextStore* textStore, RECT* rect);
	delegate VOID CompositionExtSink_native(TextStore* textStore, RECT* rect);

	CompositionSink_native^ sink_comp;
	CompositionExtSink_native^ sink_ext;
#pragma endregion
public:
	delegate VOID CompositionSink_cli(refCompositionEventArgs^ comp);
	delegate VOID CompositionExtSink_cli(System::IntPtr rect);
	event CompositionSink_cli^ eventComposition;
	event CompositionExtSink_cli^ eventGetCompExt;

	CompositionHandler(TextStore* ts);
	VOID onComposition(ITfContextOwnerCompositionSink* sink, CompositionEventArgs* comp);
	VOID onCompositionExt(TextStore* textStore, RECT* rect);
};
