#include "pch.h"
#include "TextEditSink.h"

HRESULT __stdcall TextEditSink::OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord)
{
	return S_OK;
}

TF_COM_QUERYINF(TextEditSink, TF_COM_ASUNK(ITfTextEditSink))