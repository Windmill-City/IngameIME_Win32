#include "pch.h"
#include "UIElementSink.h"
namespace libtf {
	UIElementSink::UIElementSink(Common* common)
	{
		m_common = common;
		CComQIPtr<ITfSource> source;
		source = m_common->m_pUIElementMgr;
		HRESULT hr = source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &m_dwCookie);
		THROWHR(hr, "Failed to Advisesink");
	}

	UIElementSink::~UIElementSink()
	{
		if (m_dwCookie != TF_INVALID_COOKIE) {
			CComQIPtr<ITfSource> source;
			source = m_common->m_pUIElementMgr;
			HRESULT hr = source->UnadviseSink(m_dwCookie);
			THROWHR(hr, "Failed to Unadvisesink");
		}
	}

	HRESULT __stdcall UIElementSink::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
	{
		m_sigUIElement(new UIElementEventArgs(dwUIElementId, pbShow));
		return S_OK;
	}

	HRESULT __stdcall UIElementSink::UpdateUIElement(DWORD dwUIElementId)
	{
		m_sigUIElement(new UIElementEventArgs(UIElementState::Update, dwUIElementId));
		return S_OK;
	}

	HRESULT __stdcall UIElementSink::EndUIElement(DWORD dwUIElementId)
	{
		m_sigUIElement(new UIElementEventArgs(UIElementState::End, dwUIElementId));
		return S_OK;
	}
}