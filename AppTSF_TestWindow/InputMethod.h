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

	InputMethod();
	~InputMethod();

	VOID Initialize(HWND hWnd);
	VOID SetTextBox(TextBox* textBox);
	VOID onCommit(TextStore* textStore, std::wstring commitStr);
	VOID onCompStr(TextStore* textStore, std::wstring compStr);
	VOID onGetCompsitionExt(TextStore* textStore, RECT* rect);
	VOID onQueryInsert(TextStore* textStore, LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd);

	//TextServiceFramework
	std::unique_ptr<Application> m_App;
	std::unique_ptr <Document> m_Doc;
	std::unique_ptr <Context> m_Ctx;
	Microsoft::WRL::ComPtr<TextStore> m_TextStore;
};
