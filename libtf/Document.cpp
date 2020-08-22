#include "pch.h"
#include "Document.h"
namespace libtf {
	Document::Document(const Common* common, const HWND hWnd)
	{
		this->m_hWnd = hWnd;
		this->m_pThreadMgr = common->m_pThreadMgr;
		this->m_ClientId = common->m_ClientId;

		HRESULT hr = m_pThreadMgr->CreateDocumentMgr(&m_pDocMgr);
		THROWHR(hr, "Failed to Create DocumentMgr");
		CComPtr<ITfDocumentMgr> prevDocMgr;
		hr = m_pThreadMgr->AssociateFocus(m_hWnd, m_pDocMgr.p, &prevDocMgr);
		THROWHR(hr, "Failed to AssociateFocus");
	}

	Document::~Document()
	{
		CComPtr<ITfDocumentMgr> prevDocMgr;
		HRESULT hr = m_pThreadMgr->AssociateFocus(m_hWnd, NULL, &prevDocMgr);
		THROWHR(hr, "Failed to AssociateFocus");
		hr = m_pThreadMgr->SetFocus(NULL);
		THROWHR(hr, "Failed to SetFocus->NULL")
	}
}