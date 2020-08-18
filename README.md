# libtf
A library for text service framework
# Document
[English Version](https://github.com/Windmill-City/libtf/blob/master/docs/Doc.md) 
# Build
Set Boost Root before build

Path:TextServiceFramework\libtf\PropertySheet.props

BoostVersion boost_1_74_0

Download Boost: https://www.boost.org/
Unzip, and copy the boost path to PropertySheet.props
# How to use
## C++
```c++
//Requie to init in your UIThread
//the UIThread should be running in STA Mode|CoInitialize(NULL)
HRESULT hr = m_App->Initialize();
//Activate at your UIThread
m_App->m_pThreadMgr->Activate(&(m_App->m_ClientId));
//Associate the window
m_Doc.reset(new Document(m_App.get(), hWnd));
```
#### Events
```c++
//A textStore, you should register event to it
m_TextStore = new TextStore(hWnd);
//reg events
//IME need to get the Composition String's bounding box, for CandidateList Window drawing
m_TextStore->m_sigGetCompExt.connect(boost::bind(&InputMethod::onGetCompsitionExt, this, _1, _2));
//Get the Commited Text from IME
m_TextStore->m_sigCommitStr.connect(boost::bind(&InputMethod::onCommit, this, _1, _2));
//Update the Composition string
m_TextStore->m_sigUpdateCompStr.connect(boost::bind(&InputMethod::onCompStr, this, _1, _2));
//the Caret position of the Composition
m_TextStore->m_sigUpdateCompSel.connect(boost::bind(&InputMethod::onCompSel, this, _1, _2, _3));
```
#### Push Context
```c++
//push ctx, to allow IME input
m_Ctx.reset(new Context(m_Doc.get(), (ITextStoreACP2*)m_TextStore.p));
m_Doc->m_pDocMgr->Push(m_Ctx->m_pCtx.p);
//SetFocus when ctx is vaild, it seems that some ime just handle ITfThreadMgrEventSink::OnSetFocus, so when we push context, we need to SetFocus to update their state
m_App->m_pThreadMgr->SetFocus(m_Doc->m_pDocMgr);
```
For more infomation, see https://github.com/Windmill-City/libtf/tree/master/AppTSF_TestWindow
### UILess
if you want to use UILess mode
```c++
//Remove this
//m_App->m_pThreadMgr->Activate(&(m_App->m_ClientId));
//Use this
m_App->m_pThreadMgrEx->ActivateEx(&(m_App->m_ClientId), TF_TMAE_UIELEMENTENABLEDONLY);
//Register the UIElementSink
m_UIEleSink = new UIElementSink(m_App);
m_UIEleSink->m_sigBeginUIElement.connect(your handler here);
m_UIEleSink->m_sigUpdateUIElement.connect(your handler here);
m_UIEleSink->m_sigEndUIElement.connect(your handler here);
```
#### UIElementHandler example： https://github.com/yangyuan/meow/blob/master/src/meow-uiless/meow_textapp.cpp
```c++
HRESULT MeowTextApp::BeginUIElement(DWORD dwUIElementId, BOOL *pbShow)
{
 	if (pbShow)
	{
		*pbShow = FALSE;
		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

HRESULT MeowTextApp::UpdateUIElement(DWORD dwUIElementId)
{
	CComPtr<ITfUIElement> uiElement;
	if (SUCCEEDED(uielementmgr->GetUIElement(dwUIElementId, &uiElement)))
	{
		CComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uiElement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			candidatelist.clear();

			UINT count;
			candidatelistuielement->GetCount(&count);
			UINT pcount;
			UINT pages[10];
			candidatelistuielement->GetPageIndex(pages, 10, &pcount);

			UINT cpage;
			candidatelistuielement->GetCurrentPage(&cpage);

			UINT end = count;
			UINT start = 0;
			if (cpage != pcount - 1) {
				end = pages[cpage + 1];
			}
			start = pages[cpage];
			if (pcount == 0) {
				end = start = 0;
			}
	
			{
				for (UINT i = start; i<end; ++i)
				{
					CComBSTR candidate;
					if (SUCCEEDED(candidatelistuielement->GetString(i, &candidate)))
					{
						LPWSTR text = candidate;

						candidatelist.push_back(text);
					}
				}
			}

			

		

			InvalidateRect(hwnd, NULL, NULL);
			candidatelistuielement->GetSelection(&m_candidateSelection);
		}
	}

	return S_OK;
}

HRESULT MeowTextApp::EndUIElement(DWORD dwUIElementId)
{
	CComPtr<ITfUIElement> uielement;
	if (SUCCEEDED(uielementmgr->GetUIElement(dwUIElementId, &uielement)))
	{
		CComPtr<ITfCandidateListUIElement> candidatelistuielement;
		if (SUCCEEDED(uielement->QueryInterface(IID_ITfCandidateListUIElement, (LPVOID*)&candidatelistuielement)))
		{
			candidatelist.clear();
		}
	}
	return S_OK;
}
```
### DisableIME
1. DisableSystemKeyStrokeFeed

>By default, the TSF manager will process keystrokesand pass them to the text services.
>An application prevents this by calling this method.
>Typically, this method is called when text service input is inappropriate, for example when a menu is displayed.
>Calls to this method are cumulative, so every call to this method requires a subsequent call to ITfConfigureSystemKeystrokeFeed::EnableSystemKeystrokeFeed.

>So we should use a bool to prevent multiple disable here
```c++
Application->m_pCfgSysKeyFeed->DisableSystemKeystrokeFeed();
```

**Won't cancel candidate list window if you are composing**
```c++
Context->m_pCtxOwnerCompServices->TerminateComposition(NULL);//pass NULL to terminate all composition
```
2. Pop all the context
```c++
//Pop all the context
DocumentMgr->Pop(TF_POPF_ALL)
```
3. Set TextStore ReadOnly
```c++
	/*
	Can be zero or:
  	Zero means enable read/write
  
	TS_SD_READONLY  // if set, document is read only; writes will fail
	TS_SD_LOADING   // if set, document is loading, expect additional inserts
	*/
  m_TextStore->m_status.dwDynamicFlags = TS_SD_READONLY;
  m_Ctx->m_pCtxOwnerServices->OnStatusChange(TS_SD_READONLY);
```
4. SetFocus to a NULL DocumentMgr
```c++
//Not suggest
ITfThreadMgr->AssociateFocus(m_hWnd, NULL, &prevDocMgr);
```
## C#
Use Appwrapper
```c#
//Need to init in STA UIThread
AppWrapper appWrapper = new AppWrapper();
appWrapper.Initialize(form.Handle, ActivateMode.DEFAULT);
//UIElementOnly
//appWrapper.Initialize(form.Handle, ActivateMode.UIELEMENTENABLEDONLY);

//register events
appWrapper.eventCommit += AppWrapper_eventCommit;
appWrapper.eventCompStr += AppWrapper_eventCompStr;
appWrapper.eventGetCompExt += AppWrapper_eventGetCompExt;
appWrapper.eventCompSel += AppWrapper_eventCompSel;
//you can get candidatelist here, reference to c++ example
appWrapper.eventBeginEle += AppWrapper_eventBeginEle1;
appWrapper.eventUpdateEle += AppWrapper_eventUpdateEle1;
appWrapper.eventEndEle += AppWrapper_eventEndEle1;
```
EnableIME/DisableIME
```c#
appWrapper.DisableIME();
appWrapper.EnableIME();
```
