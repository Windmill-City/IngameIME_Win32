#pragma once
#include "COMBase.h"
class TextEditSink :
	public COMBase,
	public ITfTextEditSink
{
	//ITfTextEditSink
	virtual HRESULT __stdcall OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord) override;

	//COMBase
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	TF_COM_REFS;
};
