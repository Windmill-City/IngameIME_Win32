#pragma once
#ifndef _CONTEXT_H_
#define _CONTEXT_H_

#include "Document.h"

class TFAPI Context
{
public:
	CComPtr<IUnknown> m_pTextStore;
	CComQIPtr<ITfContext> m_pCtx;
	CComQIPtr<ITfContextComposition> m_pCtxComposition;
	CComQIPtr<ITfContextOwnerServices> m_pCtxOwnerServices;
	CComQIPtr<ITfContextOwnerCompositionServices> m_pCtxOwnerCompServices;
	CComQIPtr<ITfTextEditSink> sink;
	TfEditCookie m_EditCookie = TF_INVALID_EDIT_COOKIE;

	Context(const Document* document, const HWND hWnd);
	Context(const Document* document, IUnknown* punk);
};
#endif // !_CONTEXT_H_
