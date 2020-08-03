#pragma once
#include <msctf.h>
#define TF_API  __declspec(dllexport)
class Common :
	public IUnknown
{
public:
	ITfThreadMgr* pThreadMgr;
	TfClientId client_id;

	ITfDocumentMgr* activeDocMgr;
	ITfContext* activeContext;

	~Common();
	TF_API virtual HRESULT _stdcall Initialize();

	//IUnknown
	TF_API virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	TF_API virtual ULONG __stdcall AddRef(void) override;
	TF_API virtual ULONG __stdcall Release(void) override;
private:
	ULONG reference;
};
