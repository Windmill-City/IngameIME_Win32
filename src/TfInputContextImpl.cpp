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
    ComPtr<ITfDocumentMgr> prevDocumentMgr;
    CHECK_HR(threadMgr->AssociateFocus(hWnd, emptyDocMgr.get(), &prevDocumentMgr));

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
    COM_HR_BEGIN(S_OK);
    if (activated) setActivated(false);

    if (owner) owner->UnadviseSink();
    if (h_comp) h_comp->UnadviseSink();
    if (h_mode) h_mode->UnadviseSink();

    if (docMgr) CHECK_HR(docMgr->Pop(TF_POPF_ALL));

    if (clientId != TF_CLIENTID_NULL)
    {
        CHECK_HR(threadMgr->Deactivate());
        clientId = TF_CLIENTID_NULL;
    }
    COM_HR_END();
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

    if (activated == this->activated) return;

    if (ctx)
    {
        ComQIPtr<ITfContextOwnerCompositionServices> services(IID_ITfContextOwnerCompositionServices, ctx);
        // Pass Null to terminate all the composition
        services->TerminateComposition(NULL);
    }

    ComPtr<ITfDocumentMgr> prevDocumentMgr;
    if (activated)
    {
        CHECK_HR(threadMgr->AssociateFocus(hWnd, docMgr.get(), &prevDocumentMgr));
        // Notify on activated
        this->InputModeCallbackHolder::runCallback(h_mode->inputMode);
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
} // namespace IngameIME::tf
