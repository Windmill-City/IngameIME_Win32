#include "pch.h"
#include "tf_application.h"

Application::~Application()
{
	Common::~Common();
	if (m_ClientId != TF_CLIENTID_NULL)
		m_pThreadMgr->Deactivate();
}

HRESULT _stdcall Application::Initialize()
{
	HRESULT hr = Common::Initialize();
	RETURNHR(hr);

	hr = m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr);
	RETURNHR(hr);

	m_pThreadMgrEx = m_pThreadMgr;

	m_pUIElementMgr = m_pThreadMgr;

	m_pCfgSysKeyFeed = m_pThreadMgr;

	m_pKeyMgr = m_pThreadMgr;

	m_pMsgPump = m_pThreadMgr;

	return hr;
}