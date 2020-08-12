#pragma once
#include "tf_common.h"
class Document
{
public:
	Microsoft::WRL::ComPtr <ITfDocumentMgr> m_pDocMgr;
	Microsoft::WRL::ComPtr <ITfThreadMgr> m_pThreadMgr;
	HWND m_hWnd;
	TfClientId m_ClientId;

	TFAPI Document(const Common* common, const HWND hWnd);
	TFAPI ~Document();
};
