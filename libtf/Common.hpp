#pragma once
#include <msctf.h>
#include <atlbase.h>
namespace libtf {
#define TFAPI  __declspec(dllexport)
#define THR_FAIL(hr, msg) if(FAILED(hr)) throw msg ## ":" + hr;
#define RET_FAIL(hr) if(FAILED(hr)) return hr;
#define RET_VOID(hr) if(FAILED(hr)) return;
	class TFAPI Common
	{
	public:
		CComQIPtr<ITfThreadMgr>				m_pThreadMgr;
		CComQIPtr<ITfThreadMgrEx>			m_pThreadMgrEx;
		CComQIPtr<ITfUIElementMgr>			m_pUIElementMgr;
		TfClientId							m_ClientId = TF_CLIENTID_NULL;

		~Common() {
			if (m_ClientId != TF_CLIENTID_NULL)
				m_pThreadMgr->Deactivate();
			m_pThreadMgr.Release();
			m_pThreadMgrEx.Release();
			m_pUIElementMgr.Release();
			m_ClientId = TF_CLIENTID_NULL;
			CoUninitialize();
		}

		virtual HRESULT _stdcall Initialize() {
			HRESULT hr = CoInitialize(NULL);
			return hr;
		}
	};
}
