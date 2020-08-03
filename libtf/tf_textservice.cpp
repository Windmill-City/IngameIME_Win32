#include "pch.h"
#include "tf_textservice.h"

HRESULT _stdcall TextService::Initialize()
{
	return S_OK;
}

HRESULT __stdcall TextService::QueryInterface(REFIID riid, void** ppvObject)
{
	if (ppvObject == NULL) return E_INVALIDARG;
	*ppvObject = NULL;

	if (IsEqualIID(IID_IUnknown, riid) || IsEqualGUID(IID_ITfTextInputProcessor, riid)) {
		*ppvObject = (ITfTextInputProcessor*)this;
	}

	if (*ppvObject)
	{
		Common::AddRef();
		return S_OK;
	}

	return E_NOINTERFACE;
}

HRESULT __stdcall TextService::Activate(ITfThreadMgr* ptim, TfClientId tid)
{
	Common::pThreadMgr = ptim;
	Common::client_id = tid;
	return S_OK;
}

HRESULT __stdcall TextService::Deactivate(void)
{
	Common::~Common();
	return S_OK;
}