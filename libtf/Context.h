#pragma once
#include "Document.h"

class Context
{
public:
	Microsoft::WRL::ComPtr <IUnknown> m_pTextStore;
	Microsoft::WRL::ComPtr <ITfContext> m_pCtx;
	Microsoft::WRL::ComPtr <ITfContextComposition> m_pCtxComposition;
	Microsoft::WRL::ComPtr <ITfContextOwnerServices> m_pCtxOwnerServices;
	Microsoft::WRL::ComPtr <ITfContextOwnerCompositionServices> m_pCtxOwnerCompServices;
	TfEditCookie m_EditCookie = TF_INVALID_EDIT_COOKIE;

	TFAPI Context(const Document* document, const HWND hWnd);
	TFAPI Context(const Document* document, IUnknown* punk);
};
