#pragma once
#include "tf_common.h"

class Application :
	public Common
{
public:
	ITfKeystrokeMgr* keyMgr;

	TF_API ~Application();

	TF_API virtual HRESULT _stdcall Initialize() override;
	TF_API virtual HRESULT _stdcall Activate();
	TF_API virtual HRESULT _stdcall Deactivate();
	TF_API virtual BOOL _stdcall onKeyDown(WPARAM wparam, LPARAM lparam, BOOL isTest);
	TF_API virtual BOOL _stdcall onKeyUp(WPARAM wparam, LPARAM lparam, BOOL isTest);
};
