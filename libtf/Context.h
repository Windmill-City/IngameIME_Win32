#pragma once

#include "Document.h"

class TFAPI Context
{
public:
	CComPtr<IUnknown> m_pTextStore;
	CComQIPtr<ITfDocumentMgr> m_pDocMgr;
	CComQIPtr<ITfContext> m_pCtx;
	CComQIPtr<ITfContextComposition> m_pCtxComposition;
	CComQIPtr<ITfContextOwnerServices> m_pCtxOwnerServices;
	CComQIPtr<ITfContextOwnerCompositionServices> m_pCtxOwnerCompServices;
	CComQIPtr<ITfTextEditSink> sink;
	TfEditCookie m_EditCookie = TF_INVALID_EDIT_COOKIE;

	Context(const Document* document, const HWND hWnd);
	Context(const Document* document, IUnknown* punk);
	~Context();
};
