#include "pch.h"
#include "Context.h"
#include "TextStore.h"
#include "ContextOwner.h"

Context::Context(Document* document) :Context(document, (ITfContextOwnerCompositionSink*)(new TextStore()))
{
}

Context::Context(Document* document, IUnknown* punk)
{
	m_pTextStore = punk;

	HRESULT hr = document->m_pDocMgr->CreateContext(document->m_Common->m_ClientId, 0, m_pTextStore, &m_pCtx, &m_EditCookie);

	m_pCtx->QueryInterface(IID_ITfContextOwnerCompositionServices, (LPVOID*)&m_pCtxOwnerCompServices);
	m_pCtx->QueryInterface(IID_ITfContextOwnerServices, (LPVOID*)&m_pCtxOwnerServices);

	if (m_pTextStore) m_pTextStore->AddRef();
}

Context::~Context()
{
	if (m_pCtxOwnerCompServices)
	{
		m_pCtxOwnerCompServices->Release();
		m_pCtxOwnerCompServices = NULL;
	}
	if (m_pCtxOwnerServices)
	{
		m_pCtxOwnerServices->Release();
		m_pCtxOwnerServices = NULL;
	}
	if (m_pCtx) {
		m_pCtx->Release();
		m_pCtx = NULL;
	}
	if (m_pTextStore) {
		m_pTextStore->Release();
		m_pTextStore = NULL;
	}
	m_EditCookie = TF_INVALID_COOKIE;
}