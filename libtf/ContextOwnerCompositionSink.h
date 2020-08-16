#pragma once

#include <boost/signals2.hpp>
#include "COMBase.h"

class TFAPI ContextOwnerCompositionSink :
	public COMBase,
	public ITfContextOwnerCompositionSink
{
private:
	typedef boost::signals2::signal<VOID(IUnknown*, ITfCompositionView* pComposition, BOOL* pfOk)> signal_StartComp;
	typedef boost::signals2::signal<VOID(IUnknown*, ITfCompositionView* pComposition, ITfRange* pRangeNew)> signal_UpdateComp;
	typedef boost::signals2::signal<VOID(IUnknown*, ITfCompositionView* pComposition)> signal_EndComp;
public:
	signal_StartComp m_sigStartComp;
	signal_UpdateComp m_sigUpdateComp;
	signal_EndComp m_sigEndComp;
	// Í¨¹ý ITfContextOwnerCompositionSink ¼Ì³Ð
	virtual HRESULT __stdcall OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override;
	virtual HRESULT __stdcall OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override;
	virtual HRESULT __stdcall OnEndComposition(ITfCompositionView* pComposition) override;

	//COMBase
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
		TF_COM_ASUNK(ITfContextOwnerCompositionSink);
		TF_COM_RETURN;
	}
	TF_COM_REFS;
};