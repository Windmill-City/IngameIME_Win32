#pragma once
#include "tf_common.h"

class Application :
	public Common
{
public:
	ITfKeystrokeMgr* keyMgr;

	TF_API ~Application();

	TF_API virtual HRESULT _stdcall Initialize() override;
};
