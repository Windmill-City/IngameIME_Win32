#include "tf/TfInputModeHandler.hpp"
#include "tf/TfCompositionHandler.hpp"
#include "tf/TfContextOwner.hpp"
#include "tf/TfInputContextImpl.hpp"

namespace IngameIME::tf
{
ContextOwner::ContextOwner(InputContextImpl* inputCtx)
    : ctx(inputCtx)
{
    COM_HR_BEGIN(S_OK);

    ComQIPtr<ITfSource> source(IID_ITfSource, inputCtx->ctx);
    CHECK_HR(source->AdviseSink(IID_ITfContextOwner, this, &cookie));

    COM_HR_END();
    COM_HR_THR();
}

ContextOwner::~ContextOwner()
{
    if (cookie != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, ctx->ctx);
        source->UnadviseSink(cookie);
        cookie = TF_INVALID_COOKIE;
    }
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp)
{
    return TS_E_NOLAYOUT;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped)
{
    InternalRect rect = this->ctx->getPreEditRect();
    *prc              = rect;
    // Map window coordinate to screen coordinate
    MapWindowPoints(ctx->hWnd, NULL, (LPPOINT)prc, 2);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetScreenExt(RECT* prc)
{
    GetWindowRect(ctx->hWnd, prc);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetStatus(TF_STATUS* pdcs)
{
    // Set to 0 means the context is editable
    pdcs->dwDynamicFlags = 0;
    // Set to 0 means the context only support single selection
    pdcs->dwStaticFlags  = 0;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetWnd(HWND* phwnd)
{
    *phwnd = ctx->hWnd;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue)
{
    pvarValue->vt = VT_EMPTY;
    return S_OK;
}
} // namespace IngameIME::tf