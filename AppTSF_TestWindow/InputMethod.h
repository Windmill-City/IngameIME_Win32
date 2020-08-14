#pragma once
#include <xstring>

#include "../libtf/tf_application.h"
#include "../libtf/Document.h"
#include "../libtf/Context.h"
#include "../libtf/TextStore.h"
#include "TextBox.h"
class InputMethod
{
public:
	TextBox* m_TextBox;
	BOOL m_IsIMEEnabled = TRUE;
	BOOL m_Initilized = FALSE;

	InputMethod();
	~InputMethod();

	VOID Initialize(HWND hWnd);
	VOID SetTextBox(TextBox* textBox);
	VOID onCommit(TextStore* textStore, const std::wstring commitStr);
	VOID onCompStr(TextStore* textStore, const std::wstring compStr);
	VOID onCompSel(TextStore* textStore, int acpStart, int acpEnd);
	VOID onGetCompsitionExt(TextStore* textStore, RECT* rect);
	VOID DisableIME();
	VOID EnableIME();

	//TextServiceFramework
	std::unique_ptr<Application> m_App;
	std::unique_ptr <Document> m_Doc;
	std::unique_ptr <Context> m_Ctx;
	CComPtr<TextStore> m_TextStore;
};
