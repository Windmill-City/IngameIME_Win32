#include "tf/TfInputContextImpl.hpp"
#include "tf/TfCompositionHandler.hpp"
#include "tf/TfContextOwner.hpp"
#include "tf/TfInputModeHandler.hpp"

namespace IngameIME::tf
{
InputContextImpl::InputContextImpl(const HWND hWnd)
    : hWnd(hWnd)
{
    COM_HR_BEGIN(S_OK);

    if (!hWnd) THR_HR(E_INVALIDARG);

    CHECK_HR(getThreadMgr(&threadMgr));

    ComQIPtr<ITfThreadMgrEx> threadMgrEx(IID_ITfThreadMgrEx, threadMgr);
    CHECK_HR(threadMgrEx->ActivateEx(&clientId, TF_TMAE_UIELEMENTENABLEDONLY));

    CHECK_HR(threadMgr->CreateDocumentMgr(&emptyDocMgr));
    CHECK_HR(threadMgr->CreateDocumentMgr(&docMgr));

    // Deactivate input method at initial
    setActivated(false);

    // ITfContext is created by CompositionHandler
    h_comp = new CompositionHandler(this);
    CHECK_HR(docMgr->Push(ctx.get()));

    owner  = new ContextOwner(this);
    h_mode = new InputModeHandler(this);

    COM_HR_END();
    COM_HR_THR();
}

InputContextImpl::~InputContextImpl()
{
    if (activated) setActivated(false);
    if (docMgr) docMgr->Pop(TF_POPF_ALL);

    owner.reset();
    h_mode.reset();
    h_comp.reset();

    if (clientId != TF_CLIENTID_NULL)
    {
        threadMgr->Deactivate();
        clientId = TF_CLIENTID_NULL;
    }
}

InputMode InputContextImpl::getInputMode()
{
    return h_mode->inputMode;
}

void InputContextImpl::setPreEditRect(const PreEditRect& rect)
{
    this->rect = rect;
}

PreEditRect InputContextImpl::getPreEditRect()
{
    return rect;
}

void InputContextImpl::setActivated(const bool activated)
{
    COM_HR_BEGIN(S_OK);

    ComPtr<ITfDocumentMgr> prevDocumentMgr;
    if (activated)
    {
        CHECK_HR(threadMgr->AssociateFocus(hWnd, docMgr.get(), &prevDocumentMgr));
    }
    else
    {
        // Focus on empty context so docMgr can deactivate input method
        CHECK_HR(threadMgr->AssociateFocus(hWnd, emptyDocMgr.get(), &prevDocumentMgr));
    }

    this->activated = activated;

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
}

bool InputContextImpl::getFullScreen() const
{
    return fullscreen;
}
} // namespace IngameIME::tf
