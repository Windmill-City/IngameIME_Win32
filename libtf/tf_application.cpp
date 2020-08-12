#include "pch.h"
#include "tf_application.h"

Application::~Application()
{
	Common::~Common();
}

HRESULT _stdcall Application::Initialize()
{
	HRESULT hr = Common::Initialize();
	RETURNHR(hr);

	hr = CoCreateInstance(CLSID_TF_ThreadMgr,
		NULL,
		CLSCTX_INPROC_SERVER,
		IID_ITfThreadMgr,
		&m_pThreadMgr);
	RETURNHR(hr);

	hr = m_pThreadMgr.As(&m_pThreadMgrEx);
	RETURNHR(hr);

	hr = m_pThreadMgr.As(&m_pUIElementMgr);
	RETURNHR(hr);

	hr = m_pThreadMgr.As(&m_pCfgSysKeyFeed);
	RETURNHR(hr);

	hr = m_pThreadMgr.As(&m_pKeyMgr);
	RETURNHR(hr);

	hr = m_pThreadMgr.As(&m_pMsgPump);
	RETURNHR(hr);

	return hr;
}