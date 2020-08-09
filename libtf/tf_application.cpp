#include "pch.h"
#include "tf_application.h"

Application::~Application()
{
	Common::~Common();
}

HRESULT _stdcall Application::Initialize()
{
	HRESULT hr = Common::Initialize();
	if (FAILED(hr)) return hr;

	hr = CoCreateInstance(CLSID_TF_ThreadMgr,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITfThreadMgr,
		(void**)&(Common::m_pThreadMgr));
	if (FAILED(hr)) return hr;

	return hr;
}