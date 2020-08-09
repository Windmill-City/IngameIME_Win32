#include "pch.h"
#include "Document.h"

Document::Document(Common* common, HWND hWnd)
{
	m_Common = common;
	m_hWnd = hWnd;

	m_Common->m_pThreadMgr->CreateDocumentMgr(&m_pDocMgr);
}

Document::~Document()
{
	if (m_pDocMgr) {
		m_pDocMgr->Release();
		m_pDocMgr = NULL;
	}
}