#pragma once

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
