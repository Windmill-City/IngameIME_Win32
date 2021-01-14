#pragma once
#include <iostream>
#include "Common.hpp"
#include "UIElementSink.hpp"
#include "CandidateListHandler.hpp"
namespace libtf {
	class TFAPI Application :
		public Common
	{
	public:
		CComQIPtr<ITfConfigureSystemKeystrokeFeed>			m_pCfgSysKeyFeed;
		CComQIPtr<ITfKeystrokeMgr>							m_pKeyMgr;
		CComQIPtr<ITfMessagePump>							m_pMsgPump;
		std::shared_ptr<UIElementSink>						m_pUIEleSink;
		std::shared_ptr<CandidateListHandler>				m_pCandidateListHandler;

		~Application() {
			m_pCfgSysKeyFeed.Release();
			m_pKeyMgr.Release();
			m_pMsgPump.Release();
			Common::~Common();
		}

		HRESULT _stdcall Initialize() override {
			RET_FAIL(Common::Initialize());
			RET_FAIL(m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER));

			m_pThreadMgrEx = m_pThreadMgr;
			m_pUIElementMgr = m_pThreadMgr;
			m_pCfgSysKeyFeed = m_pThreadMgr;
			m_pKeyMgr = m_pThreadMgr;
			m_pMsgPump = m_pThreadMgr;

			m_pUIEleSink.reset(new UIElementSink(m_pUIElementMgr));
			m_pCandidateListHandler.reset(new CandidateListHandler(m_pUIEleSink));
			return S_OK;
		}
	};
}
