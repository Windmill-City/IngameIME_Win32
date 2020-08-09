#include "pch.h"
#include "ContextOwner.h"

HRESULT __stdcall ContextOwner::GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp)
{
	return S_OK;
}

HRESULT __stdcall ContextOwner::GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped)
{
	return S_OK;
}

HRESULT __stdcall ContextOwner::GetScreenExt(RECT* prc)
{
	return S_OK;
}

HRESULT __stdcall ContextOwner::GetStatus(TF_STATUS* pdcs)
{
	pdcs->dwDynamicFlags = 0;
	pdcs->dwStaticFlags = 0;
	return S_OK;
}

HRESULT __stdcall ContextOwner::GetWnd(HWND* phwnd)
{
	return S_OK;
}

HRESULT __stdcall ContextOwner::GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue)
{
	return S_OK;
}

TF_COM_QUERYINF(ContextOwner, TF_COM_ASUNK(ITfContextOwner) TF_COM_AS(ITfContextOwnerCompositionSink));