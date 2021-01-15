#pragma once
#include <functional>
#include "COMBase.hpp"
#include "Common.hpp"
#include "CompositionEventArgs.hpp"
namespace libtf {
	class Document :
		private COMBase,
		private ITfContextOwner,
		private ITfContextOwnerCompositionSink,
		private ITfEditSession,
		private ITfTextEditSink
	{
		typedef std::function <VOID(CompositionEventArgs*)>										sig_Composition;
		typedef std::function <VOID(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped)>	sig_GetTextExt;

		CComPtr<ITfThreadMgr2>																	m_pThreadMgr;
		TfClientId																				m_clientId;
		DWORD																					m_dwCtxOwnerCookie;
		DWORD																					m_dwTextEditSinkCookie;
		BOOL																					m_fClean = FALSE;
	public:
		HWND																					m_hWnd;
		CComPtr<ITfDocumentMgr>																	m_pDocMgr;
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
		Document(CComPtrBase<ITfThreadMgr2> threadMgr, TfClientId clientId, HWND hWnd) {
			m_hWnd = hWnd;
			m_clientId = clientId;
			m_pThreadMgr = threadMgr;

			THR_FAIL(m_pThreadMgr->CreateDocumentMgr(&m_pDocMgr), "Failed to Create DocumentMgr");
			THR_FAIL(m_pDocMgr->CreateContext(clientId, 0, (ITfContextOwnerCompositionSink*)this, &m_pCtx, &m_ecTextStore), "Failed to Create Context");
			THR_FAIL(m_pDocMgr->Push(m_pCtx), "Failed to push context");

			CComPtr<ITfSource> source;
			source = m_pCtx;
			THR_FAIL(source->AdviseSink(IID_ITfContextOwner, (ITfContextOwner*)this, &m_dwCtxOwnerCookie), "Failed to advise ITfContextOwner");
			THR_FAIL(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink*)this, &m_dwTextEditSinkCookie), "Failed to advise ITfTextEditSink");
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
			source->UnadviseSink(m_dwTextEditSinkCookie);
			m_pDocMgr->Pop(TF_POPF_ALL);
			if (isFocusing())
				m_pThreadMgr->SetFocus(NULL);
		}

		HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
			COM_ASUNK(ITfContextOwnerCompositionSink);
			COM_AS(ITfContextOwner);
			COM_AS(ITfEditSession);
			COM_AS(ITfTextEditSink);
			COM_RETURN
		}
		COM_REFS

			HRESULT __stdcall OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override {
			*pfOk = TRUE;
			if (m_sigComposition) m_sigComposition(new CompositionEventArgs(CompositionState::StartComposition));
			return S_OK;
		}

		HRESULT __stdcall OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override {
			//Handle Composition strings at ITfTextEditSink
			return S_OK;
		}

		HRESULT __stdcall OnEndComposition(ITfCompositionView* pComposition) override {
			if (m_sigComposition) {
				HRESULT hr;
				m_pCtx->RequestEditSession(m_clientId, this, TF_ES_SYNC | TF_ES_READ, &hr);
				m_sigComposition(new CompositionEventArgs(CompositionState::EndComposition));
			}
			HRESULT hr;
			m_fClean = TRUE;
			hr = m_pCtx->RequestEditSession(m_clientId, this, TF_ES_ASYNC | TF_ES_READWRITE, &hr);
			if (FAILED(hr)) m_fClean = FALSE;
			return S_OK;
		}

		HRESULT __stdcall GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override {
			return S_OK;
		}

		HRESULT __stdcall GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override {
			if (m_sigGetTextExt) m_sigGetTextExt(acpStart, acpEnd, prc, pfClipped);
			MapWindowPoints(m_hWnd, NULL, (LPPOINT)prc, 2);
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

#define TEXT_BUF_SIZE 65
		HRESULT __stdcall DoEditSession(TfEditCookie ec) override {
			CComPtr<ITfRange> range;
			CComPtr<ITfRange> rangeEnd;
			RET_FAIL(m_pCtx->GetStart(ec, &range));
			RET_FAIL(m_pCtx->GetEnd(ec, &rangeEnd));
			RET_FAIL(range->ShiftEndToRange(ec, rangeEnd, TF_ANCHOR_END));//Get all the text in the context

			if (m_fClean) {//We need to clear the commited text
				RET_FAIL(range->SetText(ec, 0, NULL, 0));
				m_fClean = FALSE;
				return S_OK;
			}

			//Commited text
			WCHAR* buf = new WCHAR[TEXT_BUF_SIZE];
			ZeroMemory(buf, TEXT_BUF_SIZE * sizeof(WCHAR));
			ULONG cch;
			RET_FAIL(range->GetText(ec, 0, buf, TEXT_BUF_SIZE - 1, &cch));
			if (cch > 0)
				m_sigComposition(new CompositionEventArgs(buf));
			return S_OK;
		}

		HRESULT __stdcall OnEndEdit(ITfContext* pic, TfEditCookie ec, ITfEditRecord* pEditRecord) override {
			if (m_sigComposition)
			{
				CComPtr<ITfRange> range;
				CComPtr<ITfRange> rangeEnd;
				RET_FAIL(m_pCtx->GetStart(ec, &range));
				RET_FAIL(m_pCtx->GetEnd(ec, &rangeEnd));
				RET_FAIL(range->ShiftEndToRange(ec, rangeEnd, TF_ANCHOR_END));//Get all the text in the context

				//Composition string
				WCHAR* buf = new WCHAR[TEXT_BUF_SIZE];
				ZeroMemory(buf, TEXT_BUF_SIZE * sizeof(WCHAR));
				ULONG cch;
				RET_FAIL(range->GetText(ec, 0, buf, TEXT_BUF_SIZE - 1, &cch));

				if (cch > 0) {//after committed, the context has set to empty
					//Composition insertion point
					TF_SELECTION* sel = new TF_SELECTION[1];
					CComQIPtr<ITfRangeACP> rangeACP;
					ULONG fetched;
					RET_FAIL(m_pCtx->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &fetched));
					rangeACP = sel->range;
					LONG startACP, len;
					RET_FAIL(rangeACP->GetExtent(&startACP, &len));
					m_sigComposition(new CompositionEventArgs(buf, startACP));
				}
			}
			return S_OK;
		}
	};
}