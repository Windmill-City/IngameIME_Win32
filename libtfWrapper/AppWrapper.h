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
using namespace libtf;
public ref class AppWrapper
{
public:
	bool m_IsIMEEnabled = true;
	bool m_Initilized;

	AppWrapper();
	~AppWrapper();

	VOID Initialize(System::IntPtr handle, ActivateMode activateMode);
	VOID DisableIME();
	VOID EnableIME();

	//TextServiceFramework
	Application* m_App;
	Document* m_Doc;
	Context* m_Ctx;
	TextStore* m_TextStore;
	UIElementSink* m_UIEleSink;
};
