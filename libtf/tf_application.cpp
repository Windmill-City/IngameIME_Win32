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
			hr = Common::pThreadMgr->QueryInterface(IID_ITfKeystrokeMgr, (void**)&keyMgr);
		}
	}
	return hr;
}

HRESULT _stdcall Application::Activate()
{
	return pThreadMgr->Activate(&(Common::client_id));
}

HRESULT _stdcall Application::Deactivate()
{
	return pThreadMgr->Deactivate();
}

BOOL _stdcall Application::onKeyDown(WPARAM wparam, LPARAM lparam, BOOL isTest)
{
	BOOL eaten = false;
	if (isTest)
		keyMgr->TestKeyDown(wparam, lparam, &eaten);
	else
		keyMgr->KeyDown(wparam, lparam, &eaten);
	return eaten;
}

BOOL _stdcall Application::onKeyUp(WPARAM wparam, LPARAM lparam, BOOL isTest)
{
	BOOL eaten = false;
	if (isTest)
		keyMgr->TestKeyUp(wparam, lparam, &eaten);
	else
		keyMgr->KeyUp(wparam, lparam, &eaten);
	return eaten;
}