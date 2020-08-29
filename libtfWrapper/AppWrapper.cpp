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

VOID AppWrapper::PumpMessage()
{
	MSG msg;
	BOOL    fResult = TRUE;
	while (SUCCEEDED(m_App->m_pMsgPump->PeekMessage(&msg, m_hWnd, 0, 0, PM_REMOVE, &fResult)) && fResult)
	{
		BOOL    fEaten;
		if (WM_KEYDOWN == msg.message)
		{
			// does an ime want it?
			if (m_App->m_pKeyMgr->TestKeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
				m_App->m_pKeyMgr->KeyDown(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
			{
				continue;
			}
		}
		else if (WM_KEYUP == msg.message)
		{
			// does an ime want it?
			if (m_App->m_pKeyMgr->TestKeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten &&
				m_App->m_pKeyMgr->KeyUp(msg.wParam, msg.lParam, &fEaten) == S_OK && fEaten)
			{
				continue;
			}
		}

		if (WM_QUIT == msg.message)
		{
			PostMessage(m_hWnd, msg.message, msg.wParam, msg.lParam);
			return;
		}

		if (fResult)
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	PostMessage(m_hWnd, WM_NULL, 0, 0);//Prevent window from waiting msg
}

VOID AppWrapper::Initialize(System::IntPtr handle, ActivateMode activateMode)
{
	HWND hWnd = (HWND)handle.ToPointer();
	m_hWnd = hWnd;
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

VOID AppWrapper::DisableIME()
{
	if (m_IsIMEEnabled) {
		m_IsIMEEnabled = false;
		m_TextStore->m_status.dwDynamicFlags = TS_SD_READONLY;//Make textStore Readonly, prevent IME input
		m_Ctx->m_pCtxOwnerServices->OnStatusChange(m_TextStore->m_status.dwDynamicFlags);
		m_Ctx->m_pCtxOwnerCompServices->TerminateComposition(NULL);//pass NULL to terminate all composition
	}
}

VOID AppWrapper::EnableIME()
{
	if (!m_IsIMEEnabled) {
		m_IsIMEEnabled = true;
		m_TextStore->m_status.dwDynamicFlags = 0;//Set to 0, enable IME input
		m_Ctx->m_pCtxOwnerServices->OnStatusChange(m_TextStore->m_status.dwDynamicFlags);
	}
}