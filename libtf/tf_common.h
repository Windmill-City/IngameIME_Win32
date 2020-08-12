#pragma once
#include <vector>
#include <msctf.h>
#include <wrl/client.h>
#define TFAPI  __declspec(dllexport)
#define THROWHR(hr, msg) if(FAILED(hr)) throw msg ## ":" + hr;
#define RETURNHR(hr) if(FAILED(hr)) return hr;
class Common
{
public:
	Microsoft::WRL::ComPtr <ITfThreadMgr> m_pThreadMgr;
	Microsoft::WRL::ComPtr <ITfThreadMgrEx> m_pThreadMgrEx;
	Microsoft::WRL::ComPtr <ITfUIElementMgr> m_pUIElementMgr;
	TfClientId m_ClientId = TF_CLIENTID_NULL;

	~Common();
	TFAPI virtual HRESULT _stdcall Initialize();
};
