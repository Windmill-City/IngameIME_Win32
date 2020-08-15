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
	DisableIME();
	if (m_TextStore) m_TextStore->Release();
	if (m_App)
		if (m_App->m_ClientId != TF_CLIENTID_NULL)
			m_App->m_pThreadMgr->Deactivate();
}

VOID AppWrapper::Initialize(System::IntPtr handle, ActivateMode activateMode)
{
	HWND hWnd = (HWND)handle.ToPointer();
	//MS Pinyin cant open candidate window when using normal active with ITfContextOwnerCompositionSink
	//Should activate as TF_TMAE_UIELEMENTENABLEDONLY
	m_App->m_pThreadMgrEx->ActivateEx(&(m_App->m_ClientId), (DWORD)activateMode);
	m_Doc = new Document(m_App, hWnd);

	m_TextStore = new TextStore(hWnd);
	m_TextStore->AddRef();

	//reg events
	using namespace System::Runtime::InteropServices;
	m_TextStore->m_sigGetCompExt.connect(reinterpret_cast<GetCompExtCallback>(Marshal::GetFunctionPointerForDelegate(gcnew GetCompsitionExtDelegate(this, &AppWrapper::onGetCompsitionExt)).ToPointer()));
	m_TextStore->m_sigCommitStr.connect(reinterpret_cast<CommitCallback>(Marshal::GetFunctionPointerForDelegate(gcnew CommitDelegate(this, &AppWrapper::onCommit)).ToPointer()));
	m_TextStore->m_sigUpdateCompStr.connect(reinterpret_cast<CompStrCallback>(Marshal::GetFunctionPointerForDelegate(gcnew CompStrDelegate(this, &AppWrapper::onCompStr)).ToPointer()));
	m_TextStore->m_sigUpdateCompSel.connect(reinterpret_cast<CompSelCallback>(Marshal::GetFunctionPointerForDelegate(gcnew CompSelDelegate(this, &AppWrapper::onCompSel)).ToPointer()));

	m_Ctx = new Context(m_Doc, (ITextStoreACP2*)m_TextStore);
	m_UIEleSink = new UIElementSink(m_Ctx);
	m_UIEleSink->m_sigBeginUIElement.connect(reinterpret_cast<BeginUIEleCallback>(Marshal::GetFunctionPointerForDelegate(gcnew BeginUIEleDelegate(this, &AppWrapper::onBeginUIEle)).ToPointer()));
	m_UIEleSink->m_sigUpdateUIElement.connect(reinterpret_cast<UpdateUIEleCallback>(Marshal::GetFunctionPointerForDelegate(gcnew UpdateUIEleDelegate(this, &AppWrapper::onUpdateUIEle)).ToPointer()));
	m_UIEleSink->m_sigEndUIElement.connect(reinterpret_cast<EndUIEleCallback>(Marshal::GetFunctionPointerForDelegate(gcnew EndUIEleDelegate(this, &AppWrapper::onEndUIEle)).ToPointer()));

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
VOID AppWrapper::onCommit(TextStore* textStore, const std::wstring commitStr)
{
	eventCommit((IntPtr)textStore, marshal_as<String^>(commitStr));
}

VOID AppWrapper::onCompStr(TextStore* textStore, const  std::wstring compStr)
{
	eventCompStr((IntPtr)textStore, marshal_as<String^>(compStr));
}

VOID AppWrapper::onCompSel(TextStore* textStore, int acpStart, int acpEnd)
{
	eventCompSel((IntPtr)textStore, acpStart, acpEnd);
}

VOID AppWrapper::onGetCompsitionExt(TextStore* textStore, RECT* rect)
{
	refRECT^ rRect = gcnew refRECT();
	eventGetCompExt((IntPtr)textStore, rRect);
	rect->left = rRect->left;
	rect->top = rRect->top;
	rect->right = rRect->right;
	rect->bottom = rRect->bottom;
}

VOID AppWrapper::onBeginUIEle(DWORD dwUIElementId, BOOL* pbShow)
{
	bool^ show = gcnew System::Boolean();
	eventBeginEle(dwUIElementId, show);
	*pbShow = *show;
}

VOID AppWrapper::onUpdateUIEle(DWORD dwUIElementId)
{
	eventUpdateEle(dwUIElementId);
}

VOID AppWrapper::onEndUIEle(DWORD dwUIElementId)
{
	eventEndEle(dwUIElementId);
}

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