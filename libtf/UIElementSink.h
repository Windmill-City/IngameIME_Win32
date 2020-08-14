#pragma once
#include <boost/signals2.hpp>
#include "COMBase.h"

class UIElementSink :
	public COMBase,
	public ITfUIElementSink
{
public:
	typedef boost::signals2::signal<VOID(DWORD dwUIElementId, BOOL* pbShow)> signal_BeginUIElement;
	typedef boost::signals2::signal<VOID(DWORD dwUIElementId)> signal_UpdateUIElement;
	typedef boost::signals2::signal<VOID(DWORD dwUIElementId)> signal_EndUIElement;
	// Inherited via ITfUIElementSink
	virtual HRESULT __stdcall BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override;
	virtual HRESULT __stdcall UpdateUIElement(DWORD dwUIElementId) override;
	virtual HRESULT __stdcall EndUIElement(DWORD dwUIElementId) override;

	// Inherited via COMBase
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
		TF_COM_ASUNK(ITfUIElementSink);
		TF_COM_RETURN;
	}
	TF_COM_REFS;
};
