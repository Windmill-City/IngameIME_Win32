#pragma once

#include <vector>
#include <msctf.h>
#include <atlbase.h>
#define TFAPI  __declspec(dllexport)
#define THROWHR(hr, msg) if(FAILED(hr)) throw msg ## ":" + hr;
#define RETURNHR(hr) if(FAILED(hr)) return hr;
class TFAPI Common
{
public:
	CComQIPtr<ITfThreadMgr> m_pThreadMgr;
	CComQIPtr<ITfThreadMgrEx> m_pThreadMgrEx;
	CComQIPtr<ITfUIElementMgr> m_pUIElementMgr;
	TfClientId m_ClientId = TF_CLIENTID_NULL;

	~Common();
	virtual HRESULT _stdcall Initialize();
};
