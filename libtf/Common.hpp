#pragma once
#pragma warning(disable: 4251)
#include <msctf.h>
#include <atlbase.h>
#include <thread>
namespace libtf {
#define TF_API  __declspec(dllexport)
#define THR_FAIL(hr, msg) if(FAILED(hr)) throw msg;
#define RET_FAIL(hr) if(FAILED(hr)) return hr;
#define RET_VOID(hr) if(FAILED(hr)) return;
	class Common
	{
		std::thread::id						m_threadId;
	public:
		CComQIPtr<ITfThreadMgr2>			m_pThreadMgr;
		CComQIPtr<ITfThreadMgrEx>			m_pThreadMgrEx;
		CComQIPtr<ITfCompartmentMgr>		m_pCompartmentMgr;
		CComQIPtr<ITfUIElementMgr>			m_pUIElementMgr;
		TfClientId							m_ClientId = TF_CLIENTID_NULL;

		~Common() {
			if (m_ClientId != TF_CLIENTID_NULL)
				m_pThreadMgr->Deactivate();
			m_ClientId = TF_CLIENTID_NULL;
			m_threadId = std::thread::id();
		}

		virtual HRESULT _stdcall Initialize() {
			m_threadId = std::this_thread::get_id();
			return S_OK;
		}

		VOID AssertThread() {
			if (std::this_thread::get_id() != m_threadId)
				throw "Call from invaild thread";
		}
	};
}
