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
typedef void (*CompSelCallback)(TextStore* textStore, int acpStart, int acpEnd);
typedef void (*CommitCallback)(TextStore* textStore, const  std::wstring commitStr);

public ref struct refRECT
{
	int left;
	int top;
	int right;
	int bottom;
};

public ref class AppWrapper
{
public:
	bool m_IsIMEEnabled = true;
	bool m_Initilized;

	//c++ event handler
	delegate VOID CommitDelegate(TextStore* textStore, const std::wstring commitStr);
	delegate VOID CompStrDelegate(TextStore* textStore, const std::wstring compStr);
	delegate VOID CompSelDelegate(TextStore* textStore, int acpStart, int acpEnd);
	delegate VOID GetCompsitionExtDelegate(TextStore* textStore, RECT* rect);
	//C# event handler
	delegate void CommitEventHandler(System::IntPtr source, System::String^ str);
	delegate void CompStrEventHandler(System::IntPtr source, System::String^ str);
	delegate void CompSelEventHandler(System::IntPtr source, int acpStart, int acpEnd);
	delegate void GetCompsitionExtEventHandler(System::IntPtr source, refRECT^ rRect);

	event CommitEventHandler^ eventCommit;
	event CompStrEventHandler^ eventCompStr;
	event CompSelEventHandler^ eventCompSel;
	event GetCompsitionExtEventHandler^ eventGetCompExt;

	AppWrapper();
	~AppWrapper();

	VOID Initialize(System::IntPtr handle);

	VOID onCommit(TextStore* textStore, const std::wstring commitStr);
	VOID onCompStr(TextStore* textStore, const  std::wstring compStr);
	VOID onCompSel(TextStore* textStore, int acpStart, int acpEnd);
	VOID onGetCompsitionExt(TextStore* textStore, RECT* rect);

	VOID DisableIME();
	VOID EnableIME();

	//TextServiceFramework
	Application* m_App;
	Document* m_Doc;
	Context* m_Ctx;
	TextStore* m_TextStore;
};
