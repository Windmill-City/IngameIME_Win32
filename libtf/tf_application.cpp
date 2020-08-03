#include "pch.h"
#include "tf_application.h"

Application::~Application()
{
	if (keyMgr) {
		keyMgr->Release();
		keyMgr = NULL;
	}
	Common::~Common();
}

HRESULT _stdcall Application::Initialize()
{
	HRESULT hr = Common::Initialize();
	if (SUCCEEDED(hr)) {
		hr = CoCreateInstance(CLSID_TF_ThreadMgr,
			NULL,
			CLSCTX_INPROC_SERVER,
			IID_ITfThreadMgr,
			(void**)&(Common::pThreadMgr));
		if (SUCCEEDED(hr)) {
			hr = Common::pThreadMgr->Activate(&(Common::client_id));
		}
	}

	return hr;
}