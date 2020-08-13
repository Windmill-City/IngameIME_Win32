#include "TestItem2.h"

TestItem2::TestItem2()
{
}

HRESULT __stdcall TestItem2::AdviseSink(REFIID riid, IUnknown* punk, DWORD dwMask)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::UnadviseSink(IUnknown* punk)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::RequestLock(DWORD dwLockFlags, HRESULT* phrSession)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetStatus(TS_STATUS* pdcs)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::QueryInsert(LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetSelection(ULONG ulIndex, ULONG ulCount, TS_SELECTION_ACP* pSelection, ULONG* pcFetched)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::SetSelection(ULONG ulCount, const TS_SELECTION_ACP* pSelection)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetText(LONG acpStart, LONG acpEnd, WCHAR* pchPlain, ULONG cchPlainReq, ULONG* pcchPlainRet, TS_RUNINFO* prgRunInfo, ULONG cRunInfoReq, ULONG* pcRunInfoRet, LONG* pacpNext)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::SetText(DWORD dwFlags, LONG acpStart, LONG acpEnd, const WCHAR* pchText, ULONG cch, TS_TEXTCHANGE* pChange)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetFormattedText(LONG acpStart, LONG acpEnd, IDataObject** ppDataObject)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetEmbedded(LONG acpPos, REFGUID rguidService, REFIID riid, IUnknown** ppunk)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::QueryInsertEmbedded(const GUID* pguidService, const FORMATETC* pFormatEtc, BOOL* pfInsertable)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::InsertEmbedded(DWORD dwFlags, LONG acpStart, LONG acpEnd, IDataObject* pDataObject, TS_TEXTCHANGE* pChange)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::InsertTextAtSelection(DWORD dwFlags, const WCHAR* pchText, ULONG cch, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::InsertEmbeddedAtSelection(DWORD dwFlags, IDataObject* pDataObject, LONG* pacpStart, LONG* pacpEnd, TS_TEXTCHANGE* pChange)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::RequestSupportedAttrs(DWORD dwFlags, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::RequestAttrsAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::RequestAttrsTransitioningAtPosition(LONG acpPos, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::FindNextAttrTransition(LONG acpStart, LONG acpHalt, ULONG cFilterAttrs, const TS_ATTRID* paFilterAttrs, DWORD dwFlags, LONG* pacpNext, BOOL* pfFound, LONG* plFoundOffset)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::RetrieveRequestedAttrs(ULONG ulCount, TS_ATTRVAL* paAttrVals, ULONG* pcFetched)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetEndACP(LONG* pacp)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetActiveView(TsViewCookie* pvcView)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetACPFromPoint(TsViewCookie vcView, const POINT* ptScreen, DWORD dwFlags, LONG* pacp)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetTextExt(TsViewCookie vcView, LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped)
{
	return E_NOTIMPL;
}

HRESULT __stdcall TestItem2::GetScreenExt(TsViewCookie vcView, RECT* prc)
{
	return E_NOTIMPL;
}

HRESULT TestItem2::_ClearAdviseSink(PADVISE_SINK pAdviseSink)
{
	return E_NOTIMPL;
}

BOOL TestItem2::_LockDocument(DWORD dwLockFlags)
{
	return 0;
}

void TestItem2::_UnlockDocument()
{
}

BOOL TestItem2::_IsLocked(DWORD dwLockType)
{
	return 0;
}

TF_COM_QUERYINF(TestItem2, TF_COM_ASUNK(ITextStoreACP2) TF_COM_AS(ITfContextOwnerCompositionSink));