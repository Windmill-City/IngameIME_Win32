#pragma once
#include <functional>
#include "COMBase.hpp"
#include "CompositionEventArgs.hpp"
namespace libtf {
	class TFAPI ContextOwnerCompositionSink :
		public COMBase,
		public ITfContextOwnerCompositionSink
	{
		typedef std::function<VOID(ITfContextOwnerCompositionSink*, CompositionEventArgs*)> signal_Comp;
	public:
		signal_Comp				m_sigComposition = [](ITfContextOwnerCompositionSink*, CompositionEventArgs*) {};

		HRESULT __stdcall OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override
		{
			*pfOk = TRUE;//should always true
			m_sigComposition(this, new CompositionEventArgs(CompositionState::StartComposition));
			return S_OK;
		}
		HRESULT __stdcall OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override {
			m_sigComposition(this, new CompositionEventArgs(CompositionState::Composing));
			return S_OK;
		}
		HRESULT __stdcall OnEndComposition(ITfCompositionView* pComposition) override {
			m_sigComposition(this, new CompositionEventArgs(CompositionState::EndComposition));
			return S_OK;
		}

		virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
			COM_ASUNK(ITfContextOwnerCompositionSink);
			COM_RETURN;
		}
	};
}