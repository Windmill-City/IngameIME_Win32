#pragma once
#include <xstring>

#include "../libtf/tf_application.h"
#include "../libtf/Document.h"
#include "../libtf/Context.h"
#include "../libtf/TextStore.h"

#define THROWHR(hr,desc) \
if (FAILED(hr))\
	throw gcnew System::Exception(desc##":" + hr);
#define PIN(x,type) pin_ptr<type> p##x = &x;

typedef void (*GetCompExtCallback)(TextStore* ts, RECT* rect);
typedef void (*CompStrCallback)(TextStore* textStore, const  std::wstring compStr);
typedef void (*CommitCallback)(TextStore* textStore, const  std::wstring commitStr);

ref class AppWrapper
{
public:
	BOOL m_IsIMEEnabled = TRUE;
	BOOL m_Initilized = FALSE;

	AppWrapper();
	~AppWrapper();

	VOID Initialize(HWND hWnd);
	//event handler
	delegate VOID CommitDelegate(TextStore* textStore, const std::wstring commitStr);
	delegate VOID CompStrDelegate(TextStore* textStore, const std::wstring compStr);
	delegate VOID GetCompsitionExtDelegate(TextStore* textStore, RECT* rect);

	VOID onCommit(TextStore* textStore, const std::wstring commitStr);
	VOID onCompStr(TextStore* textStore, const  std::wstring compStr);
	VOID onGetCompsitionExt(TextStore* textStore, RECT* rect);

	VOID DisableIME();
	VOID EnableIME();

	//TextServiceFramework
	Application* m_App;
	Document* m_Doc;
	Context* m_Ctx;
	TextStore* m_TextStore;
};
