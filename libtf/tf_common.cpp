#include "pch.h"
#include "tf_common.h"

Common::~Common()
{
	if (m_pThreadMgr)
	{
		m_pThreadMgr->Release();
		m_pThreadMgr = NULL;
	}
	m_ClientId = 0;
	CoUninitialize();
}

HRESULT _stdcall Common::Initialize()
{
	return CoInitialize(NULL);
}