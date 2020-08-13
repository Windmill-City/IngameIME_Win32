#pragma once
#ifndef _TEXTSTORE_H_
#define _TEXTSTORE_H_

#include <xstring>

#include <queue>
#include <boost/signals2.hpp>

#include "tf_common.h"
#include "ContextOwnerCompositionSink.h"

#define EDIT_VIEW_COOKIE    0

typedef struct
{
	IUnknown* punkID;
	ITextStoreACPSink* pTextStoreACPSink;
	DWORD                   dwMask;
}ADVISE_SINK, * PADVISE_SINK;

typedef struct
{
	LONG acpStart;
	LONG acpEnd;
}ACP, * PACP;

class TFAPI TextStore :
	public ContextOwnerCompositionSink,
	public ITextStoreACP2
{
private:
	HWND                    m_hWnd;
	//TextStore
	LONG                    m_acpStart;
	LONG                    m_acpEnd;
	ULONG                   m_cchOldLength;
	BOOL                    m_fInterimChar;
	TsActiveSelEnd          m_ActiveSelEnd;
	std::wstring			m_StoredStr;
	//TextStoreSink
	ADVISE_SINK             m_AdviseSink;
	ITextStoreACPServices* m_pServices;
	BOOL                    m_fNotify;
	//DocLock
	BOOL                    m_fLocked;
	DWORD                   m_dwLockType;
	std::queue<DWORD>		m_queuedLockReq;
	//TextBox
	TS_STATUS               m_status;
	BOOL                    m_fLayoutChanged;
	//Composition
	std::wstring			m_CompStr;
public:
	typedef boost::signals2::signal<VOID(TextStore*, RECT*)> signal_GetCompExt;
	typedef signal_GetCompExt::slot_type type_GetCompExt;
	typedef boost::signals2::signal<VOID(TextStore*, const std::wstring)> signal_UpdateCompStr;
	typedef signal_UpdateCompStr::slot_type type_UpdateCompStr;
	typedef boost::signals2::signal<VOID(TextStore*, const std::wstring)> signal_CommitStr;
	typedef signal_CommitStr::slot_type type_CommitStr;

	BOOL					m_Commit;
	BOOL					m_Composing;
	LONG					m_CommitStart;
	LONG					m_CommitEnd;
	LONG					m_CompStart;
	LONG					m_CompEnd;

	//event
	signal_GetCompExt		m_sigGetCompExt;
	signal_UpdateCompStr	m_sigUpdateCompStr;
	signal_CommitStr		m_sigCommitStr;

	TextStore(HWND hWnd);
	HWND GetWnd();

	// 通过 ITextStoreACP2 继承
	virtual HRESULT __stdcall AdviseSink(REFIID riid, IUnknown* punk, DWORD dwMask) override;
	virtual HRESULT __stdcall UnadviseSink(IUnknown* punk) override;
	virtual HRESULT __stdcall RequestLock(DWORD dwLockFlags, HRESULT* phrSession) override;
	virtual HRESULT __stdcall GetStatus(TS_STATUS* pdcs) override;
	virtual HRESULT __stdcall QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd) override;
	virtual HRESULT __stdcall GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP* pSelection, ULONG* pcFetched) override;
	virtual HRESULT __stdcall SetSelection(ULONG ulCount, const TS_SELECTION_ACP* pSelection) override;
	virtual HRESULT __stdcall GetText(LONG acpStart, LONG acpEnd, WCHAR* pchPlain, ULONG cchPlainReq, ULONG* pcchPlainRet, TS_RUNINFO* prgRunInfo, ULONG cRunInfoReq, ULONG* pcRunInfoRet, LONG* pacpNext) override;
	virtual HRESULT __stdcall SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR* pchText, ULONG cch, TS_TEXTCHANGE* pChange) override;
	virtual HRESULT __stdcall GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject** ppDataObject) override;
	virtual HRESULT __stdcall GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown** ppunk) override;
	virtual HRESULT __stdcall QueryInsertEmbedded(const GUID* pguidService, const FORMATETC* pFormatEtc, BOOL* pfInsertable) override;
	virtual HRESULT __stdcall InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject* pDataObject, TS_TEXTCHANGE* pChange) override;
	virtual HRESULT __stdcall InsertTextAtSelection(DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange) override;
	virtual HRESULT __stdcall InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject* pDataObject, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange) override;
	virtual HRESULT __stdcall RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs) override;
	virtual HRESULT __stdcall RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags) override;
	virtual HRESULT __stdcall RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags) override;
	virtual HRESULT __stdcall FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags, LONG* pacpNext, BOOL* pfFound, LONG* plFoundOffset) override;
	virtual HRESULT __stdcall RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL* paAttrVals, ULONG* pcFetched) override;
	virtual HRESULT __stdcall GetEndACP(LONG* pacp) override;
	virtual HRESULT __stdcall GetActiveView(TsViewCookie* pvcView) override;
	virtual HRESULT __stdcall GetACPFromPoint(TsViewCookie vcView, const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override;
	virtual HRESULT __stdcall GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override;
	virtual HRESULT __stdcall GetScreenExt(TsViewCookie vcView, RECT* prc) override;

	// 通过 COMBase 继承
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override;
	TF_COM_REFS;
private:
	//TextStoreSink
	HRESULT _ClearAdviseSink(PADVISE_SINK pAdviseSink);
	//DocLock
	BOOL _LockDocument(DWORD dwLockFlags);
	void _UnlockDocument();
	BOOL _IsLocked(DWORD dwLockType);
};
#endif // !_TEXTSTORE_H_
