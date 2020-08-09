#include "pch.h"
#include "tf_textservice.h"

HRESULT __stdcall TextService::Activate(ITfThreadMgr* ptim, TfClientId tid)
{
	return S_OK;
}

HRESULT __stdcall TextService::Deactivate(void)
{
	return S_OK;
}

TF_COM_QUERYINF(TextService, TF_COM_ASUNK(ITfTextInputProcessor))