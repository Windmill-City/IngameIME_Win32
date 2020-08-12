#include "pch.h"
#include "tf_common.h"

Common::~Common()
{
	m_ClientId = TF_CLIENTID_NULL;
	CoUninitialize();
}

HRESULT _stdcall Common::Initialize()
{
	return CoInitialize(NULL);
}