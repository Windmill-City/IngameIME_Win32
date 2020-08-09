#pragma once
#include "COMBase.h"
#include "tf_common.h"
class TextService :
	public COMBase,
	public ITfTextInputProcessor
{
public:
	//ITfTextInputProcessor
	virtual HRESULT __stdcall Activate(ITfThreadMgr* ptim, TfClientId tid) override;
	virtual HRESULT __stdcall Deactivate(void) override;

	//COMBase
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	TF_COM_REFS;
};
