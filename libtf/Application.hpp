#pragma once
#include <iostream>
#include "COMBase.hpp"
#include "Common.hpp"
#include "UIElementSink.hpp"
#include "CandidateListHandler.hpp"
namespace libtf {
	class TFAPI Application :
		public Common,
		public COMBase,
		private ITfCompartmentEventSink
	{
		typedef std::function<VOID(BOOL)>					sig_AlphaMode;

		DWORD												m_dwConversionModeCookie;
	public:
		CComQIPtr<ITfConfigureSystemKeystrokeFeed>			m_pCfgSysKeyFeed;
		CComQIPtr<ITfKeystrokeMgr>							m_pKeyMgr;
		CComQIPtr<ITfMessagePump>							m_pMsgPump;

		std::shared_ptr<UIElementSink>						m_pUIEleSink;
		std::shared_ptr<CandidateListHandler>				m_pCandidateListHandler;

		CComPtr<ITfCompartment>								m_pConversionMode;

		sig_AlphaMode										m_sigAlphaMode = [](BOOL) {};

		~Application() {
			CComPtr<ITfSource> source;
			source = m_pConversionMode;
			THR_FAIL(source->UnadviseSink(m_dwConversionModeCookie), "Failed to Unadvise AlphaMode sink");
		}

		/// <summary>
		/// Initialize at UI-Thread
		/// </summary>
		/// <returns>
		/// HRESULT of Initialize
		/// </returns>
		HRESULT _stdcall Initialize() override {
			RET_FAIL(Common::Initialize());
			RET_FAIL(m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER));

			m_pThreadMgrEx = m_pThreadMgr;
			m_pCompartmentMgr = m_pThreadMgr;
			m_pUIElementMgr = m_pThreadMgr;
			m_pCfgSysKeyFeed = m_pThreadMgr;
			m_pKeyMgr = m_pThreadMgr;
			m_pMsgPump = m_pThreadMgr;

			m_pThreadMgrEx->ActivateEx(&m_ClientId, TF_TMAE_UIELEMENTENABLEDONLY);

			m_pUIEleSink.reset(new UIElementSink(m_pUIElementMgr));
			m_pCandidateListHandler.reset(new CandidateListHandler(m_pUIElementMgr, m_pUIEleSink));
			RET_FAIL(m_pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &m_pConversionMode));
			CComPtr<ITfSource> source;
			source = m_pConversionMode;
			RET_FAIL(source->AdviseSink(IID_ITfCompartmentEventSink, (ITfCompartmentEventSink*)this, &m_dwConversionModeCookie));
			return S_OK;
		}

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
