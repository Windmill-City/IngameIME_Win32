#include "pch.h"
#include "UIElementSink.h"

UIElementSink::UIElementSink(Application* app)
{
	m_app = app;
	CComQIPtr<ITfSource> source;
	source = m_app->m_pUIElementMgr;
	HRESULT hr = source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &m_dwCookie);
	THROWHR(hr, "Failed to Advisesink");
}

UIElementSink::~UIElementSink()
{
	if (m_dwCookie != TF_INVALID_COOKIE) {
		CComQIPtr<ITfSource> source;
		source = m_app->m_pUIElementMgr;
		HRESULT hr = source->UnadviseSink(m_dwCookie);
		THROWHR(hr, "Failed to Unadvisesink");
	}
}

HRESULT __stdcall UIElementSink::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
{
	m_sigBeginUIElement(dwUIElementId, pbShow);
	return S_OK;
}

HRESULT __stdcall UIElementSink::UpdateUIElement(DWORD dwUIElementId)
{
	m_sigUpdateUIElement(dwUIElementId);
	return S_OK;
}

HRESULT __stdcall UIElementSink::EndUIElement(DWORD dwUIElementId)
{
	m_sigEndUIElement(dwUIElementId);
	return S_OK;
}