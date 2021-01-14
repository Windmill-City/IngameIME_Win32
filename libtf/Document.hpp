#pragma once
#include <functional>
#include "COMBase.hpp"
#include "Common.hpp"
#include "CompositionEventArgs.hpp"
namespace libtf {
	class Document:
		public COMBase,
		public ITfContextOwner,
		public ITfContextOwnerCompositionSink
	{
		typedef std::function <VOID(CompositionEventArgs*)>										sig_Composition;
		typedef std::function <VOID(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped)>	sig_GetTextExt;

		CComQIPtr<ITfThreadMgr2>																	m_pThreadMgr;
		TfClientId																				m_clientId;
		DWORD																					m_dwCtxOwnerCookie;
	public:
		HWND																					m_hWnd;
		CComQIPtr<ITfDocumentMgr>																m_pDocMgr;
		CComPtr<ITfContext>																		m_pCtx;
		TfEditCookie																			m_ecTextStore;

		sig_Composition																			m_sigComposition = [](CompositionEventArgs*) {};
		sig_GetTextExt																			m_sigGetTextExt = [](LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) {};

		/// <summary>
		/// Create Document for the window
		/// </summary>
		/// <param name="threadMgr">Get from Application</param>
		/// <param name="clientId">Get from Application</param>
		/// <param name="hWnd">The window's handle</param>
		Document(CComPtrBase<ITfThreadMgr2> threadMgr, TfClientId clientId , HWND hWnd) {
			m_hWnd = hWnd;
			m_clientId = clientId;
			m_pThreadMgr = threadMgr;

			THR_FAIL(m_pThreadMgr->CreateDocumentMgr(&m_pDocMgr), "Failed to Create DocumentMgr");
			THR_FAIL(m_pDocMgr->CreateContext(clientId, 0, (ITfContextOwnerCompositionSink*)this, &m_pCtx, &m_ecTextStore), "Failed to Create Context");
			THR_FAIL(m_pDocMgr->Push(m_pCtx), "Failed to push context");

			CComPtr<ITfSource> source;
			source = m_pCtx;
			THR_FAIL(source->AdviseSink(IID_ITfContextOwner, (ITfContextOwner*)this, &m_dwCtxOwnerCookie), "Failed to advise ITfContextOwner");
		}

		/// <summary>
		/// Focus when correspond window gets input focus
		/// </summary>
		VOID Focus() {
			m_pThreadMgr->SetFocus(m_pDocMgr.p);
		}

		BOOL isFocusing() {
			CComPtr<ITfDocumentMgr> curDocMgr;
			m_pThreadMgr->GetFocus(&curDocMgr);
			return curDocMgr == m_pDocMgr;
		}

		~Document() {
			CComPtr<ITfSource> source;
			source = m_pCtx;
			source->UnadviseSink(m_dwCtxOwnerCookie);
			m_pDocMgr->Pop(TF_POPF_ALL);
			if(isFocusing())
				THR_FAIL(m_pThreadMgr->SetFocus(NULL), "Failed to SetFocus to NULL")
		}

		HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
			COM_ASUNK(ITfContextOwnerCompositionSink);
			COM_AS(ITfContextOwner);
			COM_RETURN
		}
		COM_REFS

		HRESULT __stdcall OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override {
			*pfOk = TRUE;
			if (m_sigComposition) m_sigComposition(new CompositionEventArgs(CompositionState::StartComposition));
			return S_OK;
		}

#define TEXT_BUF_SIZE 25
		HRESULT __stdcall OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override {
			if (m_sigComposition) {
				CComPtr<ITfRange> range;
				RET_FAIL(pComposition->GetRange(&range));
				WCHAR* buf = new WCHAR[TEXT_BUF_SIZE];
				ZeroMemory(buf, TEXT_BUF_SIZE * sizeof(WCHAR));
				ULONG cch;
				range->GetText(m_ecTextStore, 0, buf, TEXT_BUF_SIZE, &cch);
				m_sigComposition(new CompositionEventArgs(buf, 0));
			}
			return S_OK;
		}

		HRESULT __stdcall OnEndComposition(ITfCompositionView* pComposition) override {
			if (m_sigComposition) {
				CComPtr<ITfRange> range;
				RET_FAIL(pComposition->GetRange(&range));
				WCHAR* buf = new WCHAR[TEXT_BUF_SIZE];
				ZeroMemory(buf, TEXT_BUF_SIZE * sizeof(WCHAR));
				ULONG cch;
				range->GetText(m_ecTextStore, 0, buf, TEXT_BUF_SIZE, &cch);
				if (cch > 0)
					m_sigComposition(new CompositionEventArgs(buf));
				m_sigComposition(new CompositionEventArgs(CompositionState::EndComposition));
			}
			return S_OK;
		}

		HRESULT __stdcall GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override {
			return S_OK;
		}

		HRESULT __stdcall GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override {
			if (m_sigGetTextExt) m_sigGetTextExt(acpStart, acpEnd, prc, pfClipped);
			return S_OK;
		}

		HRESULT __stdcall GetScreenExt(RECT* prc) override {
			GetWindowRect(m_hWnd, prc);
			return S_OK;
		}

		HRESULT __stdcall GetStatus(TF_STATUS* pdcs) override {
			pdcs->dwDynamicFlags = 0;
			pdcs->dwStaticFlags = 0;
			return S_OK;
		}

		HRESULT __stdcall GetWnd(HWND* phwnd) override {
			phwnd = &m_hWnd;
			return S_OK;
		}

		HRESULT __stdcall GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue) override {
			return S_OK;
		}
	};
}