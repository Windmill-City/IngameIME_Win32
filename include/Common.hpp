#pragma once
#pragma warning(disable : 4251)
#include <msctf.h>
#include <atlbase.h>
#include <thread>
namespace libtf
{
#define TF_API __declspec(dllexport)
#define THR_FAIL(hr, msg) \
	if (FAILED(hr))       \
		throw msg;
#define RET_FAIL(hr) \
	if (FAILED(hr))  \
		return hr;
#define RET_VOID(hr) \
	if (FAILED(hr))  \
		return;
	class Common
	{
	public:
		CComQIPtr<ITfThreadMgr2> m_pThreadMgr;
		CComQIPtr<ITfThreadMgrEx> m_pThreadMgrEx;
		CComQIPtr<ITfCompartmentMgr> m_pCompartmentMgr;
		CComQIPtr<ITfUIElementMgr> m_pUIElementMgr;

		~Common()
		{
			CoUninitialize();
		}

		virtual HRESULT _stdcall Initialize()
		{
			return CoInitialize(NULL);
		}
	};
}
