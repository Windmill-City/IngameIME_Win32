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
		&m_pThreadMgr);
	if (FAILED(hr)) return hr;

	hr = m_pThreadMgr.As(&m_pThreadMgrEx);
	if (FAILED(hr)) return hr;

	hr = m_pThreadMgr.As(&m_pUIElementMgr);
	if (FAILED(hr)) return hr;

	hr = m_pThreadMgr.As(&m_pCfgSysKeyFeed);

	return hr;
}