#pragma once
#include <iostream>
#include "COMBase.hpp"
#include "Common.hpp"
#include "Document.hpp"
#include "UIElementSink.hpp"
#include "CandidateListHandler.hpp"
namespace libtf {
	class TF_API Application :
		public Common,
		private COMBase,
		private ITfCompartmentEventSink
	{
		typedef std::function<VOID(BOOL)>					sig_AlphaMode;

		DWORD												m_dwConversionModeCookie = 0;
		BOOL												m_fKeyStrokeFeedState = TRUE;
	public:
		std::shared_ptr<UIElementSink>						m_pUIEleSink;
		std::shared_ptr<CandidateListHandler>				m_pCandidateListHandler;

		CComPtr<ITfCompartment>								m_pConversionMode;

		sig_AlphaMode										m_sigAlphaMode = [](BOOL) {};

		~Application() {
			CComPtr<ITfSource> source;
			source = m_pConversionMode;
			source->UnadviseSink(m_dwConversionModeCookie);
		}

		/// <summary>
		/// Initialize at UI-Thread
		/// </summary>
		HRESULT _stdcall Initialize() override {
			RET_FAIL(Common::Initialize());
			RET_FAIL(m_pThreadMgr.CoCreateInstance(CLSID_TF_ThreadMgr, NULL, CLSCTX_INPROC_SERVER));

			m_pThreadMgrEx = m_pThreadMgr;
			m_pCompartmentMgr = m_pThreadMgr;
			m_pUIElementMgr = m_pThreadMgr;

			m_pThreadMgrEx->ActivateEx(&m_ClientId, TF_TMAE_UIELEMENTENABLEDONLY);

			m_pUIEleSink.reset(new UIElementSink(m_pUIElementMgr));
			m_pCandidateListHandler.reset(new CandidateListHandler(m_pUIElementMgr, m_pUIEleSink));
			RET_FAIL(m_pCompartmentMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &m_pConversionMode));
			CComPtr<ITfSource> source;
			source = m_pConversionMode;
			RET_FAIL(source->AdviseSink(IID_ITfCompartmentEventSink, (ITfCompartmentEventSink*)this, &m_dwConversionModeCookie));
			return S_OK;
		}

		BOOL KeyStrokeFeedState() {
			return m_fKeyStrokeFeedState;
		}
		
		/// <summary>
		/// Set current thread's key handleing status
		/// if suspended, IME wont process keys
		/// </summary>
		/// <param name="state"></param>
		VOID setKeyStrokeFeedState(BOOL state) {
			AssertThread();
			m_fKeyStrokeFeedState = state;
			if (m_fKeyStrokeFeedState)
				m_pThreadMgr->ResumeKeystrokeHandling();
			else
				m_pThreadMgr->SuspendKeystrokeHandling();
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
				m_sigAlphaMode(val.lVal == TF_CONVERSIONMODE_ALPHANUMERIC);
			}
			return S_OK;
		}
	};
}
