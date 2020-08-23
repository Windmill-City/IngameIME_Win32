#include "pch.h"
#include<boost/bind.hpp>
#include "AppWrapper.h"
AppWrapper::AppWrapper()
{
	m_App = new Application();
	HRESULT hr = m_App->Initialize();
	THROWHR(hr, "Failed to Initialize Application");
}

AppWrapper::~AppWrapper()
{
	if (m_Ctx)
	{
		DisableIME();//in case start a new composition
		delete m_Ctx;
	}
	if (m_TextStore) m_TextStore->Release();
	if (m_Doc)
		delete m_Doc;
	EnableIME();//Default state is Enabled
	if (m_App)
		delete m_App;
}

CompositionHandler^ AppWrapper::GetCompHandler()
{
	return gcnew CompositionHandler(m_TextStore);
}

CandidateListWrapper^ AppWrapper::GetCandWapper()
{
	return gcnew CandidateListWrapper(m_UIEleSink, m_App);
}

VOID AppWrapper::Initialize(System::IntPtr handle, ActivateMode activateMode)
{
	HWND hWnd = (HWND)handle.ToPointer();
	//MS Pinyin cant open candidate window when using normal active with ITfContextOwnerCompositionSink
	//Should activate as TF_TMAE_UIELEMENTENABLEDONLY
	m_App->m_pThreadMgrEx->ActivateEx(&(m_App->m_ClientId), (DWORD)activateMode);
	m_UIEleSink = new UIElementSink(m_App);
	m_Doc = new Document(m_App, hWnd);

	m_TextStore = new TextStore(hWnd);
	m_TextStore->AddRef();

	m_Ctx = new Context(m_Doc, (ITextStoreACP2*)m_TextStore);
	//push ctx
	DisableIME();//Disable input before push, in case start composition
	m_Doc->m_pDocMgr->Push(m_Ctx->m_pCtx.p);
	//SetFocus when ctx is vaild, it seems that some ime just handle ITfThreadMgrEventSink::OnSetFocus, so when we push context, we need to SetFocus to update their state
	m_App->m_pThreadMgr->SetFocus(m_Doc->m_pDocMgr);
	m_Initilized = true;
}

#include <msclr\marshal_cppstd.h>

using namespace System;
using namespace msclr::interop;

VOID AppWrapper::DisableIME()
{
	if (m_IsIMEEnabled) {
		m_IsIMEEnabled = false;
		/*
		By default, the TSF manager will process keystrokesand pass them to the text services.
		An application prevents this by calling this method.
		Typically, this method is called when text service input is inappropriate, for example when a menu is displayed.
		Calls to this method are cumulative, so every call to this method requires a subsequent call to ITfConfigureSystemKeystrokeFeed::EnableSystemKeystrokeFeed.

		So we use a bool to prevent multiple disable here
		*/
		m_App->m_pCfgSysKeyFeed->DisableSystemKeystrokeFeed();
		m_Ctx->m_pCtxOwnerCompServices->TerminateComposition(NULL);//pass NULL to terminate all composition
	}
}

VOID AppWrapper::EnableIME()
{
	if (!m_IsIMEEnabled) {
		m_IsIMEEnabled = true;
		m_App->m_pCfgSysKeyFeed->EnableSystemKeystrokeFeed();
	}
}