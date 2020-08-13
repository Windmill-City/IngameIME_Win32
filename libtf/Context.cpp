#include "pch.h"
#include "Context.h"
#include "TextStore.h"
#include "TextEditSink.h"

Context::Context(const Document* document, const HWND hWnd) :Context(document, (ITextStoreACP2*)new TextStore(hWnd))
{
}

Context::Context(const Document* document, IUnknown* punk)
{
	m_pTextStore = punk;

	HRESULT hr = document->m_pDocMgr->CreateContext(document->m_ClientId, 0, m_pTextStore.p, &m_pCtx, &m_EditCookie);
	THROWHR(hr, "Failed to Create ITfContext");

	m_pCtxOwnerCompServices = m_pCtx;

	m_pCtxOwnerServices = m_pCtx;

	m_pCtxComposition = m_pCtx;

	sink = new TextEditSink(this);
}