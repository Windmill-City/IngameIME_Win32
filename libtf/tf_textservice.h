#pragma once
#include "tf_common.h"
class TextService :
	public Common,
	public ITfTextInputProcessor
{
public:
	virtual HRESULT _stdcall Initialize() override;

	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;

	//ITfTextInputProcessor
	virtual HRESULT __stdcall Activate(ITfThreadMgr* ptim, TfClientId tid) override;
	virtual HRESULT __stdcall Deactivate(void) override;
};
