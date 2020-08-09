#pragma once
#include <vector>
#include <msctf.h>
#define TFAPI  __declspec(dllexport)
class Common
{
public:
	ITfThreadMgr* m_pThreadMgr;
	TfClientId m_ClientId;

	~Common();
	TFAPI virtual HRESULT _stdcall Initialize();
};
