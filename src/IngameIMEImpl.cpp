#include <stdarg.h>

#include "ImmCompositionImpl.hpp"
#include "ImmIngameIMEImpl.hpp"

#include "TfCompositionImpl.hpp"
#include "TfIngameIMEImpl.hpp"

libtf::InputContextImpl::InputContextImpl(HWND hWnd)
{
    COM_HR_BEGIN(S_OK);

    if (!(this->hWnd = reinterpret_cast<HWND>(hWnd))) THR_HR(E_INVALIDARG);
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

libimm::InputContextImpl::InputContextImpl(HWND hWnd) : hWnd(hWnd)
{
    comp = std::make_shared<CompositionImpl>(this);

    // Reset to default context
    ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
    ctx = ImmAssociateContext(hWnd, NULL);

    prevProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)InputContextImpl::WndProc);
}

#include <versionhelpers.h>
IngameIME::Global& IngameIME::Global::getInstance()
{
    thread_local IngameIME::Global& Instance = IsWindows8OrGreater() ? (IngameIME::Global&)*new libtf::GlobalImpl() :
                                                                       (IngameIME::Global&)*new libimm::GlobalImpl();
    return Instance;
}