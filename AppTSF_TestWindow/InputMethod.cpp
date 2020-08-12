#include<boost/bind.hpp>   
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
	m_TextStore->m_sigQueryIns.connect(boost::bind(&InputMethod::onQueryInsert, this, _1, _2, _3, _4, _5, _6));
	m_TextStore->m_sigGetCompExt.connect(boost::bind(&InputMethod::onGetCompsitionExt, this, _1, _2));
	m_TextStore->m_sigCommitStr.connect(boost::bind(&InputMethod::onCommit, this, _1, _2));
	m_TextStore->m_sigUpdateCompStr.connect(boost::bind(&InputMethod::onCompStr, this, _1, _2));
	//push ctx
	m_Ctx.reset(new Context(m_Doc.get(), (ITextStoreACP2*)m_TextStore.Get()));
	m_Doc->m_pDocMgr->Push(m_Ctx->m_pCtx.Get());
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
	if (!textBox) return;
	textBox->m_CompText = L"";
	SetRectEmpty(&textBox->m_rectComp);
}

VOID InputMethod::onCommit(TextStore* textStore, std::wstring commitStr)
{
	if (textStore != m_TextStore.Get() || !m_TextBox) return;
	m_TextBox->m_Text.append(commitStr);
	InvalidateRect(textStore->GetWnd(), NULL, NULL);
}

VOID InputMethod::onCompStr(TextStore* textStore, std::wstring compStr)
{
	if (textStore != m_TextStore.Get() || !m_TextBox) return;
	m_TextBox->m_CompText = compStr;
	InvalidateRect(textStore->GetWnd(), NULL, NULL);
}

VOID InputMethod::onGetCompsitionExt(TextStore* textStore, RECT* rect)
{
	if (textStore != m_TextStore.Get() || !m_TextBox) return;
	m_TextBox->GetCompExt(rect);
}

VOID InputMethod::onQueryInsert(TextStore* textStore, LONG acpTestStart, LONG acpTestEnd, ULONG cch, LONG* pacpResultStart, LONG* pacpResultEnd)
{
	if (textStore != m_TextStore.Get() || !m_TextBox) return;
	m_TextBox->QueryInsert(cch, pacpResultStart, pacpResultEnd);
}