#pragma once
#include "tf_common.h"
#include "Document.h"
class Application :
	public Common
{
public:
	TFAPI ~Application();
	TFAPI virtual HRESULT _stdcall Initialize() override;
};
