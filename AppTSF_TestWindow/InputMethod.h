#pragma once
//#define UILESS
#include <xstring>

#include "../libtf/tf_application.h"
#include "../libtf/Document.h"
#include "../libtf/Context.h"
#include "../libtf/TextStore.h"
#include "../libtf/UIElementSink.h"
#include "../libtf/CandidateListHandler.h"
#include "TextBox.h"
using namespace libtf;
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
	VOID onComposition(ITfContextOwnerCompositionSink* sink, CompositionEventArgs* comp);
	VOID onGetCompsitionExt(TextStore* textStore, RECT* rect);
#ifdef UILESS
	VOID onCandidateList(CandidateList* list);
#endif // UILESS

	VOID DisableIME();
	VOID EnableIME();

	//TextServiceFramework
	std::unique_ptr<Application> m_App;
	std::unique_ptr <Document> m_Doc;
	std::unique_ptr <Context> m_Ctx;
	CComPtr<TextStore> m_TextStore;
#ifdef UILESS
	CComPtr<UIElementSink> m_UIEleSink;
	std::unique_ptr<CandidateListHandler> m_CandListHandler;
#endif // UILESS
};
