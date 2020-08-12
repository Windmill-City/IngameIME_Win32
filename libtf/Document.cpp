#include "pch.h"
#include "Document.h"

Document::Document(const Common* common, const HWND hWnd)
{
	this->m_hWnd = hWnd;
	this->m_pThreadMgr = common->m_pThreadMgr;
	this->m_ClientId = common->m_ClientId;

	m_pThreadMgr->CreateDocumentMgr(&m_pDocMgr);
	Microsoft::WRL::ComPtr<ITfDocumentMgr> prevDocMgr;
	m_pThreadMgr->AssociateFocus(m_hWnd, m_pDocMgr.Get(), &prevDocMgr);
}

Document::~Document()
{
	Microsoft::WRL::ComPtr<ITfDocumentMgr> prevDocMgr;
	m_pThreadMgr->AssociateFocus(m_hWnd, NULL, &prevDocMgr);
}