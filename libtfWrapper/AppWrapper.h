#pragma once
#include <xstring>

#include "../libtf/tf_application.h"
#include "../libtf/Document.h"
#include "../libtf/Context.h"
#include "../libtf/TextStore.h"
#include "../libtf/UIElementSink.h"

#define THROWHR(hr,desc) \
if (FAILED(hr))\
	throw gcnew System::Exception(desc##":" + hr);
#define PIN(x,type) pin_ptr<type> p##x = &x;

typedef void (*GetCompExtCallback)(TextStore* ts, RECT* rect);
typedef void (*CompStrCallback)(TextStore* textStore, const  std::wstring compStr);
typedef void (*CompSelCallback)(TextStore* textStore, int acpStart, int acpEnd);
typedef void (*CommitCallback)(TextStore* textStore, const  std::wstring commitStr);

typedef void (*BeginUIEleCallback)(DWORD dwUIElementId, BOOL* pbShow);
typedef void (*UpdateUIEleCallback)(DWORD dwUIElementId);
typedef void (*EndUIEleCallback)(DWORD dwUIElementId);

public enum class ActivateMode
{
	DEFAULT = 0,
	NOACTIVATETIP = TF_TMAE_NOACTIVATETIP,
	SECUREMODE = TF_TMAE_SECUREMODE,
	UIELEMENTENABLEDONLY = TF_TMAE_UIELEMENTENABLEDONLY,
	COMLESS = TF_TMAE_COMLESS,
	WOW16 = TF_TMAE_WOW16,
	NOACTIVATEKEYBOARDLAYOUT = TF_TMAE_NOACTIVATEKEYBOARDLAYOUT,
	CONSOLE = TF_TMAE_CONSOLE
};

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

	delegate VOID BeginUIEleDelegate(DWORD dwUIElementId, BOOL* pbShow);
	delegate VOID UpdateUIEleDelegate(DWORD dwUIElementId);
	delegate VOID EndUIEleDelegate(DWORD dwUIElementId);
	//C# event handler
	delegate void CommitEventHandler(System::IntPtr source, System::String^ str);
	delegate void CompStrEventHandler(System::IntPtr source, System::String^ str);
	delegate void CompSelEventHandler(System::IntPtr source, int acpStart, int acpEnd);
	delegate void GetCompsitionExtEventHandler(System::IntPtr source, refRECT^ rRect);

	delegate void BeginUIEleHandler(unsigned long UIElementId, bool^% Show);
	delegate void UpdateUIEleHandler(unsigned long UIElementId);
	delegate void EndUIEleHandler(unsigned long UIElementId);

	event CommitEventHandler^ eventCommit;
	event CompStrEventHandler^ eventCompStr;
	event CompSelEventHandler^ eventCompSel;
	event GetCompsitionExtEventHandler^ eventGetCompExt;

	event BeginUIEleHandler^ eventBeginEle;
	event UpdateUIEleHandler^ eventUpdateEle;
	event EndUIEleHandler^ eventEndEle;

	AppWrapper();
	~AppWrapper();

	VOID Initialize(System::IntPtr handle, ActivateMode activateMode);

	VOID onCommit(TextStore* textStore, const std::wstring commitStr);
	VOID onCompStr(TextStore* textStore, const  std::wstring compStr);
	VOID onCompSel(TextStore* textStore, int acpStart, int acpEnd);
	VOID onGetCompsitionExt(TextStore* textStore, RECT* rect);

	VOID onBeginUIEle(DWORD dwUIElementId, BOOL* pbShow);
	VOID onUpdateUIEle(DWORD dwUIElementId);
	VOID onEndUIEle(DWORD dwUIElementId);

	VOID DisableIME();
	VOID EnableIME();

	//TextServiceFramework
	Application* m_App;
	Document* m_Doc;
	Context* m_Ctx;
	TextStore* m_TextStore;
	UIElementSink* m_UIEleSink;
};
