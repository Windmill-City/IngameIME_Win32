#pragma once
#include "Document.h"

class Context
{
public:
	IUnknown* m_pTextStore;

	ITfContext* m_pCtx;
	ITfContextOwnerServices* m_pCtxOwnerServices;
	ITfContextOwnerCompositionServices* m_pCtxOwnerCompServices;
	TfEditCookie m_EditCookie = TF_INVALID_COOKIE;

	TFAPI Context(Document* document);
	TFAPI Context(Document* document, IUnknown* punk);
	TFAPI ~Context();
};
