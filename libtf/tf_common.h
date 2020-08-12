#pragma once
#ifndef _TF_COMMON_H_
#define _TF_COMMON_H_

#include <vector>
#include <msctf.h>
#include <atlbase.h>
#define TFAPI  __declspec(dllexport)
#define THROWHR(hr, msg) if(FAILED(hr)) throw msg ## ":" + hr;
#define RETURNHR(hr) if(FAILED(hr)) return hr;
class Common
{
public:
	CComQIPtr<ITfThreadMgr> m_pThreadMgr;
	CComQIPtr<ITfThreadMgrEx> m_pThreadMgrEx;
	CComQIPtr<ITfUIElementMgr> m_pUIElementMgr;
	TfClientId m_ClientId = TF_CLIENTID_NULL;

	~Common();
	TFAPI virtual HRESULT _stdcall Initialize();
};
#endif // !_TF_COMMON_H_
