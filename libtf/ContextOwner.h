#pragma once

#include "ContextOwnerCompositionSink.h"
class TFAPI ContextOwner :
	public ITfContextOwner,
	public ContextOwnerCompositionSink
{
public:
	//ITfContextOwner
	virtual HRESULT __stdcall GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override;
	virtual HRESULT __stdcall GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override;
	virtual HRESULT __stdcall GetScreenExt(RECT* prc) override;
	virtual HRESULT __stdcall GetStatus(TF_STATUS* pdcs) override;
	virtual HRESULT __stdcall GetWnd(HWND* phwnd) override;
	virtual HRESULT __stdcall GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue) override;

	//COMBase
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
		TF_COM_ASUNK(ITfContextOwner);
		TF_COM_AS(ITfContextOwnerCompositionSink);
		TF_COM_RETURN;
	}
	TF_COM_REFS;
};
