#include "pch.h"
#include "ContextOwnerCompositionSink.h"
namespace libtf {
	HRESULT __stdcall ContextOwnerCompositionSink::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
	{
		*pfOk = TRUE;//should always true
		m_sigComposition(this, new CompositionEventArgs(CompositionState::StartComposition));
		return S_OK;
	}

	HRESULT __stdcall ContextOwnerCompositionSink::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
	{
		m_sigComposition(this, new CompositionEventArgs(CompositionState::Composing));
		return S_OK;
	}

	HRESULT __stdcall ContextOwnerCompositionSink::OnEndComposition(ITfCompositionView* pComposition)
	{
		m_sigComposition(this, new CompositionEventArgs(CompositionState::EndComposition));
		return S_OK;
	}
}