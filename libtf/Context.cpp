#include "pch.h"
#include "Context.h"
#include "TextStore.h"
#include "TextEditSink.h"

Microsoft::WRL::ComPtr<TextEditSink> sink;
Context::Context(const Document* document, const HWND hWnd) :Context(document, (ITextStoreACP2*)new TextStore(hWnd))
{
}

Context::Context(const Document* document, IUnknown* punk)
{
	m_pTextStore = punk;

	HRESULT hr = document->m_pDocMgr->CreateContext(document->m_ClientId, 0, m_pTextStore.Get(), &m_pCtx, &m_EditCookie);

	m_pCtx.As(&m_pCtxOwnerCompServices);
	m_pCtx.As(&m_pCtxOwnerServices);
	m_pCtx.As(&m_pCtxComposition);

	sink = new TextEditSink(this);
}