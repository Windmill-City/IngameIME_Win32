#pragma once
#include "tf_common.h"
#include "Document.h"
class Application :
	public Common
{
public:
	Microsoft::WRL::ComPtr<ITfConfigureSystemKeystrokeFeed> m_pCfgSysKeyFeed;
	Microsoft::WRL::ComPtr<ITfKeystrokeMgr> m_pKeyMgr;
	Microsoft::WRL::ComPtr<ITfMessagePump> m_pMsgPump;

	TFAPI ~Application();
	TFAPI virtual HRESULT _stdcall Initialize() override;
};
