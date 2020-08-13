#include "pch.h"
#include "ContextOwnerCompositionSink.h"

HRESULT __stdcall ContextOwnerCompositionSink::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
	*pfOk = TRUE;
	m_sigStartComp((ITfContextOwnerCompositionSink*)this, pComposition, pfOk);
	return S_OK;
}

HRESULT __stdcall ContextOwnerCompositionSink::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
	m_sigUpdateComp((ITfContextOwnerCompositionSink*)this, pComposition, pRangeNew);
	return S_OK;
}

HRESULT __stdcall ContextOwnerCompositionSink::OnEndComposition(ITfCompositionView* pComposition)
{
	m_sigEndComp((ITfContextOwnerCompositionSink*)this, pComposition);
	return S_OK;
}