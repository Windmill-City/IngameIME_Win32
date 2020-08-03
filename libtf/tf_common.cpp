#include "pch.h"
#include "tf_common.h"

Common::~Common()
{
	if (pThreadMgr)
	{
		pThreadMgr->Release();
		pThreadMgr = NULL;
	}
	client_id = 0;
	CoUninitialize();
}

HRESULT _stdcall Common::Initialize()
{
	return CoInitialize(NULL);
}

HRESULT __stdcall Common::QueryInterface(REFIID riid, void** ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	*ppvObject = NULL;

	if (IsEqualIID(IID_IUnknown, riid)) {
		*ppvObject = (IUnknown*)this;
	}

	if (*ppvObject)
	{
		AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

ULONG __stdcall Common::AddRef(void)
{
	return ++reference;
}

ULONG __stdcall Common::Release(void)
{
	if (--reference <= 0) {
		delete this;
		return 0;
	}
	return reference;
}