#include "tf/TfInputContextImpl.hpp"

#include "tf/TfCompositionImpl.hpp"

namespace IngameIME::tf

{

InputContextImpl::ContextOwner::ContextOwner(InputContextImpl* ctx)
    : ctx(ctx)
{
}

HRESULT STDMETHODCALLTYPE InputContextImpl::ContextOwner::GetACPFromPoint(const POINT* ptScreen,
                                                                          DWORD        dwFlags,
                                                                          LONG*        pacp)
{
    return TS_E_NOLAYOUT;
}

HRESULT STDMETHODCALLTYPE InputContextImpl::ContextOwner::GetTextExt(LONG  acpStart,
                                                                     LONG  acpEnd,
                                                                     RECT* prc,
                                                                     BOOL* pfClipped)
{
    InternalRect rect = this->ctx->comp->getPreEditRect();
    *prc              = rect;
    // Map window coordinate to screen coordinate
    MapWindowPoints(ctx->hWnd, NULL, (LPPOINT)prc, 2);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE InputContextImpl::ContextOwner::GetScreenExt(RECT* prc)
{
    GetWindowRect(ctx->hWnd, prc);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE InputContextImpl::ContextOwner::GetStatus(TF_STATUS* pdcs)
{
    // Set to 0 indicates the context is editable
    pdcs->dwDynamicFlags = 0;
    // Set to 0 indicates the context only support single selection
    pdcs->dwStaticFlags  = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE InputContextImpl::ContextOwner::GetWnd(HWND* phwnd)
{
    *phwnd = ctx->hWnd;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE InputContextImpl::ContextOwner::GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue)
{
    pvarValue->vt = VT_EMPTY;
    return S_OK;
}

InputContextImpl::InputContextImpl(const HWND hWnd)
    : hWnd(hWnd)
{
    COM_HR_BEGIN(S_OK);

    if (!this->hWnd) THR_HR(E_INVALIDARG);
    if (initialCreatorThread() != GetWindowThreadProcessId(hWnd, NULL)) THR_HR(UI_E_WRONG_THREAD);

    CHECK_HR(getThreadMgr(&threadMgr));

    ComQIPtr<ITfThreadMgrEx> threadMgrEx(IID_ITfThreadMgrEx, threadMgr);
    CHECK_HR(threadMgrEx->ActivateEx(&clientId, TF_TMAE_UIELEMENTENABLEDONLY));

    CHECK_HR(threadMgr->CreateDocumentMgr(&emptyDocMgr));
    CHECK_HR(threadMgr->CreateDocumentMgr(&docMgr));

    comp = std::make_shared<CompositionImpl>(this);
    CHECK_HR(docMgr->Push(ctx.get()));

    // Deactivate input contxt
    setActivated(false);

    ComQIPtr<ITfSource> source(IID_ITfSource, ctx);
    owner = new InputContextImpl::ContextOwner(this);
    CHECK_HR(source->AdviseSink(IID_ITfContextOwner, owner.get(), &cookie));

    COM_HR_END();
    COM_HR_THR();
}

InputContextImpl::~InputContextImpl()
{
    if (cookie != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, ctx);
        source->UnadviseSink(cookie);
        cookie = TF_INVALID_COOKIE;
        owner.reset();
    }

    if (ctx)
    {
        setActivated(false);
        comp.reset();
        docMgr->Pop(TF_POPF_ALL);
        ctx.reset();
    }

    if (docMgr)
    {
        docMgr.reset();
        emptyDocMgr.reset();
    }

    if (clientId != TF_CLIENTID_NULL)
    {
        threadMgr->Deactivate();
        threadMgr.reset();
        clientId = TF_CLIENTID_NULL;
    }
}

void InputContextImpl::setActivated(const bool activated)
{
    COM_HR_BEGIN(S_OK);

    CHECK_HR(assertCreatorThread());

    this->activated = activated;

    ComPtr<ITfDocumentMgr> prevDocumentMgr;
    if (activated)
    {
        CHECK_HR(threadMgr->AssociateFocus(hWnd, docMgr.get(), &prevDocumentMgr));
    }
    else
    {
        // Focus on empty context docMgr can deactivate input method
        CHECK_HR(threadMgr->AssociateFocus(hWnd, emptyDocMgr.get(), &prevDocumentMgr));
        // Terminate active composition
        comp->terminate();
    }

    COM_HR_END();
    COM_HR_THR();
}

bool InputContextImpl::getActivated() const
{
    return activated;
}

void InputContextImpl::setFullScreen(const bool fullscreen)
{
    this->fullscreen = fullscreen;
    if (activated) comp->terminate();
}

bool InputContextImpl::getFullScreen() const
{
    return fullscreen;
}

} // namespace IngameIME::tf
