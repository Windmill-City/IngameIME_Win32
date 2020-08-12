#pragma once
#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include "tf_common.h"
class Document
{
public:

	CComQIPtr<ITfDocumentMgr> m_pDocMgr;
	CComQIPtr<ITfThreadMgr> m_pThreadMgr;
	HWND m_hWnd;
	TfClientId m_ClientId;

	TFAPI Document(const Common* common, const HWND hWnd);
	TFAPI ~Document();
};
#endif // !_DOCUMENT_H_
