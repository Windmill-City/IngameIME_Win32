#include <boost/bind.hpp>   
#include "InputMethod.h"
InputMethod::InputMethod()
{
	m_App.reset(new Application());
	HRESULT hr = m_App->Initialize();
	if (FAILED(hr))
		throw "Failed to Initialize Application:" + hr;
}

InputMethod::~InputMethod()
{
	if (m_App) {
		if (m_App->m_ClientId != TF_CLIENTID_NULL) m_App->m_pThreadMgr->Deactivate();
	}
}

VOID InputMethod::Initialize(HWND hWnd)
{
	//MS Pinyin cant open candidate window when using normal active with ITfContextOwnerCompositionSink
	//m_App->m_pThreadMgrEx->ActivateEx(&(m_App->m_ClientId), TF_TMAE_UIELEMENTENABLEDONLY);
	m_App->m_pThreadMgr->Activate(&(m_App->m_ClientId));
	m_Doc.reset(new Document(m_App.get(), hWnd));

	m_TextStore = new TextStore(hWnd);
	//reg events
	m_TextStore->m_sigGetCompExt.connect(boost::bind(&InputMethod::onGetCompsitionExt, this, _1, _2));
	m_TextStore->m_sigCommitStr.connect(boost::bind(&InputMethod::onCommit, this, _1, _2));
	m_TextStore->m_sigUpdateCompStr.connect(boost::bind(&InputMethod::onCompStr, this, _1, _2));
	//push ctx
	m_Ctx.reset(new Context(m_Doc.get(), (ITextStoreACP2*)m_TextStore.p));
	DisableIME();//Disable input before push, in case start composition
	m_Doc->m_pDocMgr->Push(m_Ctx->m_pCtx.p);
	//SetFocus when ctx is vaild, it seems that some ime just handle ITfThreadMgrEventSink::OnSetFocus, so when we push context, we need to SetFocus to update their state
	m_App->m_pThreadMgr->SetFocus(m_Doc->m_pDocMgr);
	m_Initilized = TRUE;
}

VOID InputMethod::SetTextBox(TextBox* textBox)
{
	//Cleanup old
	if (textBox) {
		m_Ctx->m_pCtxOwnerCompServices->TerminateComposition(NULL);
		textBox->m_CompText = L"";
		SetRectEmpty(&textBox->m_rectComp);
	}
	//Cleanup new
	m_TextBox = textBox;
	if (!textBox) {
		DisableIME();
		return;
	};
	textBox->m_CompText = L"";
	SetRectEmpty(&textBox->m_rectComp);
	EnableIME();
}

VOID InputMethod::onCommit(TextStore* textStore, const std::wstring commitStr)
{
	if (textStore != m_TextStore.p || !m_TextBox) return;
	m_TextBox->m_Text.append(commitStr);
	InvalidateRect(textStore->GetWnd(), NULL, NULL);
}

VOID InputMethod::onCompStr(TextStore* textStore, const std::wstring compStr)
{
	if (textStore != m_TextStore.p || !m_TextBox) return;
	m_TextBox->m_CompText = compStr;
	InvalidateRect(textStore->GetWnd(), NULL, NULL);
}

VOID InputMethod::onGetCompsitionExt(TextStore* textStore, RECT* rect)
{
	if (textStore != m_TextStore.p || !m_TextBox) return;
	m_TextBox->GetCompExt(rect);
}

VOID InputMethod::DisableIME()
{
	if (m_IsIMEEnabled) {
		m_IsIMEEnabled = FALSE;
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

VOID InputMethod::EnableIME()
{
	if (!m_IsIMEEnabled)
		m_IsIMEEnabled = TRUE;
	m_App->m_pCfgSysKeyFeed->EnableSystemKeystrokeFeed();
}