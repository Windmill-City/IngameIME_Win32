#include "pch.h"
#include "ContextOwner.h"

HRESULT __stdcall ContextOwner::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
	*pfOk = TRUE;
	return S_OK;
}

HRESULT __stdcall ContextOwner::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
	return S_OK;
}

HRESULT __stdcall ContextOwner::OnEndComposition(ITfCompositionView* pComposition)
{
	return S_OK;
}