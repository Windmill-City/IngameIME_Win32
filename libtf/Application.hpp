#pragma once
#include <iostream>
#include "COMBase.hpp"
#include "Common.hpp"
#include "UIElementSink.hpp"
#include "CandidateListHandler.hpp"
namespace libtf {
	class TFAPI Application :
		public Common,
		private COMBase,
		private ITfCompartmentEventSink
	{
		typedef std::function<VOID(BOOL)>					sig_AlphaMode;
		DWORD												m_dwCMode;
	public:
		CComQIPtr<ITfConfigureSystemKeystrokeFeed>			m_pCfgSysKeyFeed;
		CComQIPtr<ITfKeystrokeMgr>							m_pKeyMgr;
		CComQIPtr<ITfMessagePump>							m_pMsgPump;
		std::shared_ptr<UIElementSink>						m_pUIEleSink;
		std::shared_ptr<CandidateListHandler>				m_pCandidateListHandler;
		CComPtr<ITfCompartment>								m_pConversionMode;

		sig_AlphaMode										m_sigAlphaMode = [](BOOL) {};

		~Application() {
			m_pCfgSysKeyFeed.Release();
			m_pKeyMgr.Release();
			m_pMsgPump.Release();
			m_pCandidateListHandler.reset();
			m_pUIEleSink.reset();
			CComQIPtr<ITfSource> source = m_pConversionMode.p;
			source->UnadviseSink(m_dwCMode);
			m_pConversionMode.Release();
			Common::~Common();
		}

		HRESULT _stdcall Initialize() override {
			RET_FAIL(Common::Initialize());
			RET_FAIL(m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER));

			m_pThreadMgrEx = m_pThreadMgr;
			m_pCompartmentMgr = m_pThreadMgr;
			m_pUIElementMgr = m_pThreadMgr;
			m_pCfgSysKeyFeed = m_pThreadMgr;
			m_pKeyMgr = m_pThreadMgr;
			m_pMsgPump = m_pThreadMgr;

			m_pUIEleSink.reset(new UIElementSink(m_pUIElementMgr));
			m_pCandidateListHandler.reset(new CandidateListHandler(m_pUIEleSink));
			RET_FAIL(m_pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &m_pConversionMode));
			CComQIPtr<ITfSource> source = m_pConversionMode.p;
			RET_FAIL(source->AdviseSink(IID_ITfCompartmentEventSink, (ITfCompartmentEventSink*)this, &m_dwCMode));
			return S_OK;
		}
	private:
		HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
			COM_ASUNK(ITfCompartmentEventSink);
			COM_RETURN
		}
		COM_REFS;

		HRESULT __stdcall OnChange(REFGUID rguid) override {
			if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
			{
				CComVariant val;
				RET_FAIL(m_pConversionMode->GetValue(&val));
				m_sigAlphaMode(val.lVal & TF_CONVERSIONMODE_ALPHANUMERIC);
			}
			return S_OK;
		}
	};
}
