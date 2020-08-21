#include "pch.h"
#include "tf_common.h"

Common::~Common()
{
	if (m_ClientId != TF_CLIENTID_NULL)
		m_pThreadMgr->Deactivate();
	m_pThreadMgr.Release();
	m_pThreadMgrEx.Release();
	m_pUIElementMgr.Release();
	m_ClientId = TF_CLIENTID_NULL;
	CoUninitialize();
}

HRESULT _stdcall Common::Initialize()
{
	HRESULT hr = CoInitialize(NULL);
	return hr;
}