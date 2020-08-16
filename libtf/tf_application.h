#pragma once

#include "tf_common.h"
class TFAPI Application :
	public Common
{
public:
	CComQIPtr<ITfConfigureSystemKeystrokeFeed> m_pCfgSysKeyFeed;
	CComQIPtr<ITfKeystrokeMgr> m_pKeyMgr;
	CComQIPtr<ITfMessagePump> m_pMsgPump;

	~Application();
	virtual HRESULT _stdcall Initialize() override;
};
