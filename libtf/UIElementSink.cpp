#include "pch.h"
#include "UIElementSink.h"

UIElementSink::UIElementSink(Context* ctx)
{
	m_pCtx = ctx->m_pCtx;
	CComQIPtr<ITfSource> source;
	source = ctx->m_pCtx;
	source->AdviseSink(IID_ITfUIElementSink, (ITfUIElementSink*)this, &m_dwCookie);
}

UIElementSink::~UIElementSink()
{
	if (m_dwCookie != TF_INVALID_COOKIE) {
		CComQIPtr<ITfSource> source;
		source = m_pCtx;
		HRESULT hr = source->UnadviseSink(m_dwCookie);
		THROWHR(hr, "Failed to Unadvicesink");
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