#include "tf/TfContextOwner.hpp"
#include "tf/TfCompositionHandler.hpp"
#include "tf/TfInputContextImpl.hpp"
#include "tf/TfInputModeHandler.hpp"

namespace IngameIME::tf
{
ContextOwner::ContextOwner(InputContextImpl* inputCtx)
    : inputCtx(inputCtx)
{
    COM_HR_BEGIN(S_OK);

    ComQIPtr<ITfSource> source(IID_ITfSource, inputCtx->ctx);
    CHECK_HR(source->AdviseSink(IID_ITfContextOwner, this, &cookieOwner));

    COM_HR_END();
    COM_HR_THR();
}

void ContextOwner::UnadviseSink()
{
    COM_HR_BEGIN(S_OK);
    if (cookieOwner != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, inputCtx->ctx);
        CHECK_HR(source->UnadviseSink(cookieOwner));
        cookieOwner = TF_INVALID_COOKIE;
    }
    COM_HR_END();
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp)
{
    return TS_E_NOLAYOUT;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped)
{
    InternalRect rect = inputCtx->getPreEditRect();
    *prc              = rect;
    // Map window coordinate to screen coordinate
    MapWindowPoints(inputCtx->hWnd, NULL, (LPPOINT)prc, 2);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetScreenExt(RECT* prc)
{
    GetWindowRect(inputCtx->hWnd, prc);
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
    *phwnd = inputCtx->hWnd;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE ContextOwner::GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue)
{
    pvarValue->vt = VT_EMPTY;
    return S_OK;
}
} // namespace IngameIME::tf