#pragma once
#ifndef _DOCUMENT_H_
#define _DOCUMENT_H_

#include "tf_common.h"
class TFAPI Document
{
public:

	CComQIPtr<ITfDocumentMgr> m_pDocMgr;
	CComQIPtr<ITfThreadMgr> m_pThreadMgr;
	HWND m_hWnd;
	TfClientId m_ClientId;

	Document(const Common* common, const HWND hWnd);
	~Document();
};
#endif // !_DOCUMENT_H_
