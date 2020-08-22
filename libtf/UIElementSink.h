#pragma once
#include <boost/signals2.hpp>
#include "COMBase.h"
#include "UIElementEventArgs.h"
namespace libtf {
	class TFAPI UIElementSink :
		public COMBase,
		public ITfUIElementSink
	{
	public:
		typedef boost::signals2::signal<VOID(UIElementEventArgs*)> signal_UIElement;

		signal_UIElement			m_sigUIElement;

		Common* m_common;
		DWORD							m_dwCookie = TF_INVALID_COOKIE;

		UIElementSink(Common* common);
		~UIElementSink();

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
}
