#pragma once
#include "COMBase.h"
class ContextOwner :
	public COMBase,
	public ITfContextOwner,
	public ITfContextOwnerCompositionSink
{
	//ITfContextOwner
	virtual HRESULT __stdcall GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override;
	virtual HRESULT __stdcall GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override;
	virtual HRESULT __stdcall GetScreenExt(RECT* prc) override;
	virtual HRESULT __stdcall GetStatus(TF_STATUS* pdcs) override;
	virtual HRESULT __stdcall GetWnd(HWND* phwnd) override;
	virtual HRESULT __stdcall GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue) override;

	//ITfContextOwnerCompositionSink
	virtual HRESULT __stdcall OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override;
	virtual HRESULT __stdcall OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override;
	virtual HRESULT __stdcall OnEndComposition(ITfCompositionView* pComposition) override;

	//COMBase
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	TF_COM_REFS;
};
