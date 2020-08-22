# libtf
A library for text service framework
# Document
[English Version](https://github.com/Windmill-City/libtf/blob/master/docs/Doc.md) 
# Build
Set Boost Root before build

Path:`TextServiceFramework\libtf\PropertySheet.props`

BoostVersion boost_1_74_0

Download Boost: https://www.boost.org/

Unzip, and copy the boost path to `PropertySheet.props`
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
//Get Composition Text|Caret Pos|Commit Text
m_TextStore->m_sigComposition.connect(boost::bind(&InputMethod::onComposition, this, _1, _2));
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
m_UIEleSink = new UIElementSink(m_App.get());
m_CandListHandler.reset(new CandidateListHandler(m_UIEleSink.p, m_App.get()));
//Get CandidateList info from this event
m_CandListHandler->m_sigCandidateList.connect(boost::bind(&InputMethod::onCandidateList, this, _1));
```
UIElementHandler reference： https://github.com/yangyuan/meow/blob/master/src/meow-uiless/meow_textapp.cpp
### DisableIME
1. DisableSystemKeyStrokeFeed

> By default, the TSF manager will process keystrokesand pass them to the text services.

> An application prevents this by calling this method.

> Typically, this method is called when text service input is inappropriate, for example when a menu is displayed.

> Calls to this method are cumulative, so every call to this method requires a subsequent call to ITfConfigureSystemKeystrokeFeed::EnableSystemKeystrokeFeed.

**So we should prevent multiple disable here(or any other solutions)**
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
ITfThreadMgr->AssociateFocus(m_hWnd, NULL, &prevDocMgr);
```
## C#
Use Appwrapper
```c#
//Need to init in STA UIThread
AppWrapper appWrapper = new AppWrapper();
appWrapper.Initialize(form.Handle, ActivateMode.DEFAULT);
```
### UILess
```c#
//Only activate the IME who supports UILess Mode
appWrapper.Initialize(form.Handle, ActivateMode.UIELEMENTENABLEDONLY);
```
### EnableIME/DisableIME
```c#
appWrapper.DisableIME();
appWrapper.EnableIME();
```
