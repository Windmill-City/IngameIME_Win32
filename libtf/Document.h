#pragma once
#include "tf_common.h"
class Document
{
public:
	ITfDocumentMgr* m_pDocMgr;

	Common* m_Common;
	HWND m_hWnd;

	TFAPI Document(Common* common, HWND hWnd);
	TFAPI ~Document();
};
