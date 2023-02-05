#include "tf/TfCompositionImpl.hpp"

namespace IngameIME::tf

{

CompositionImpl::CompositionHandler::CompositionHandler(CompositionImpl* comp)
    : comp(comp)
{
    eleMgr = comp->inputCtx->threadMgr;
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::OnStartComposition(ITfCompositionView* pComposition,
                                                                                  BOOL*               pfOk)
{
    if (!pfOk) return E_INVALIDARG;

    // Always allow Composition start
    *pfOk = true;

    comp->PreEditCallbackHolder::runCallback(CompositionState::Begin, nullptr);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::OnUpdateComposition(ITfCompositionView* pComposition,
                                                                                   ITfRange*           pRangeNew)
{
    // Handle preedit in ITfTextEditSink
    compView = pComposition;

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::OnEndComposition(ITfCompositionView* pComposition)
{
    compView.reset();
    comp->PreEditCallbackHolder::runCallback(CompositionState::End, nullptr);

    static HRESULT hr;
    hr = comp->inputCtx->ctx->RequestEditSession(comp->inputCtx->clientId, this, TF_ES_ASYNC | TF_ES_READWRITE, &hr);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::OnEndEdit(ITfContext*    pic,
                                                                         TfEditCookie   ec,
                                                                         ITfEditRecord* pEditRecord)
{
    COM_HR_BEGIN(S_OK);

    // No active composition
    if (!compView) return S_OK;

    ComPtr<ITfRange> preEditRange;
    CHECK_HR(compView->GetRange(&preEditRange));

    // Get preedit length
    ComQIPtr<ITfRangeACP> rangeAcp(IID_ITfRangeACP, preEditRange);
    LONG                  acpStart, len;
    CHECK_HR(rangeAcp->GetExtent(&acpStart, &len));
    ULONG preEditLen = len;
    auto  bufPreEdit = std::make_unique<WCHAR[]>(preEditLen);
    // Get preedit text
    CHECK_HR(preEditRange->GetText(ec, 0, bufPreEdit.get(), preEditLen, &preEditLen));

    // Get selection of the preedit
    TF_SELECTION     sel[1];
    ULONG            fetched;
    ComPtr<ITfRange> selRange;
    CHECK_HR(comp->inputCtx->ctx->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &fetched));
    selRange.attach(sel[0].range);
    rangeAcp = selRange;
    CHECK_HR(rangeAcp->GetExtent(&acpStart, &len));

    PreEditContext preEditCtx;
    preEditCtx.selStart = acpStart;
    preEditCtx.selEnd   = acpStart + len;
    preEditCtx.content  = std::wstring(bufPreEdit.get(), preEditLen);

    comp->PreEditCallbackHolder::runCallback(CompositionState::Update, &preEditCtx);

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::DoEditSession(TfEditCookie ec)
{
    COM_HR_BEGIN(S_OK);

    auto inputCtx = comp->inputCtx;
    auto ctx      = inputCtx->ctx;

    // Get a range which covers all the texts in the context
    ComPtr<ITfRange> fullRange;
    ComPtr<ITfRange> rangeAtEnd;
    CHECK_HR(ctx->GetStart(ec, &fullRange));
    CHECK_HR(ctx->GetEnd(ec, &rangeAtEnd));
    CHECK_HR(fullRange->ShiftEndToRange(ec, rangeAtEnd.get(), TF_ANCHOR_END));

    // It's possible that the context is empty when there is no commit
    BOOL isEmpty;
    CHECK_HR(fullRange->IsEmpty(ec, &isEmpty));
    if (isEmpty) return S_OK;

    // Get the text length
    ComQIPtr<ITfRangeACP> rangeAcp(IID_ITfRangeACP, fullRange);
    LONG                  acpStart, len;
    CHECK_HR(rangeAcp->GetExtent(&acpStart, &len));
    ULONG commitLen = len;
    auto  bufCommit = std::make_unique<WCHAR[]>(commitLen);
    // Get the commit text
    CHECK_HR(fullRange->GetText(ec, 0, bufCommit.get(), commitLen, &commitLen));
    // Clear the texts in the text store
    CHECK_HR(fullRange->SetText(ec, 0, NULL, 0));

    comp->CommitCallbackHolder::runCallback(std::wstring(bufCommit.get(), commitLen));

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
{
    COM_HR_BEGIN(S_OK);

    if (dwUIElementId == TF_INVALID_UIELEMENTID) return E_INVALIDARG;
    if (!pbShow) return E_INVALIDARG;

    auto inputCtx = comp->inputCtx;

    *pbShow = !inputCtx->fullscreen;

    ComPtr<ITfUIElement> uiEle;
    CHECK_HR(eleMgr->GetUIElement(dwUIElementId, &uiEle));

    ComQIPtr<ITfCandidateListUIElement> candEle(IID_ITfCandidateListUIElement, uiEle);
    // Check if current UIElement is CandidateListUIElement
    if (candEle)
    {
        ele   = candEle;
        eleId = dwUIElementId;
        // Handle Candidate List events
        comp->CandidateListCallbackHolder::runCallback(CandidateListState::Begin, nullptr);
    }

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::UpdateUIElement(DWORD dwUIElementId)
{
    COM_HR_BEGIN(S_OK);

    if (dwUIElementId == TF_INVALID_UIELEMENTID) return E_INVALIDARG;
    if (eleId == TF_INVALID_UIELEMENTID || dwUIElementId != eleId) return S_OK;

    // Total count of Candidates
    uint32_t totalCount;
    CHECK_HR(ele->GetCount(&totalCount));

    // How many pages?
    uint32_t pageCount;
    CHECK_HR(ele->GetPageIndex(NULL, 0, &pageCount));

    // Array of pages' start index
    auto pageStarts = std::make_unique<uint32_t[]>(pageCount);
    CHECK_HR(ele->GetPageIndex(pageStarts.get(), pageCount, &pageCount));

    // Current page's index in pageStarts
    uint32_t curPage;
    CHECK_HR(ele->GetCurrentPage(&curPage));

    uint32_t pageStart = pageStarts[curPage];
    uint32_t pageEnd   = curPage == pageCount - 1 ? totalCount : pageStarts[curPage + 1];
    uint32_t pageSize  = pageEnd - pageStart;

    CandidateListContext candCtx;

    // Currently Selected Candidate's absolute index
    UINT sel;
    CHECK_HR(ele->GetSelection(&sel));
    // Absolute index to relative index
    sel -= pageStart;
    candCtx.selection = sel;

    // Get Candidate Strings
    for (uint32_t i = pageStart; i < pageEnd; i++)
    {
        ComBSTR candidate;
        if (FAILED(ele->GetString(i, &candidate)))
            candCtx.candidates.push_back(L"[err]");
        else
            candCtx.candidates.push_back(candidate.bstr);
    }

    comp->CandidateListCallbackHolder::runCallback(CandidateListState::Update, &candCtx);

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionImpl::CompositionHandler::EndUIElement(DWORD dwUIElementId)
{
    COM_HR_BEGIN(S_OK);

    if (dwUIElementId == TF_INVALID_UIELEMENTID) return E_INVALIDARG;
    if (eleId == TF_INVALID_UIELEMENTID || dwUIElementId != eleId) return S_OK;

    eleId = TF_INVALID_UIELEMENTID;
    ele.reset();

    comp->CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);

    COM_HR_END();
    COM_HR_RET();
}

CompositionImpl::CompositionImpl(InputContextImpl* inputCtx)
    : inputCtx(inputCtx)
{
    COM_HR_BEGIN(S_OK);

    handler = new CompositionHandler(this);

    ComQIPtr<ITfUIElementMgr> eleMgr(IID_ITfUIElementMgr, inputCtx->threadMgr);
    ComQIPtr<ITfSource>       source(IID_ITfSource, eleMgr);
    CHECK_HR(source->AdviseSink(IID_ITfUIElementSink, static_cast<ITfUIElementSink*>(handler.get()), &cookieEleSink));

    // This EditCookie is useless
    TfEditCookie ec;
    CHECK_HR(inputCtx->docMgr->CreateContext(inputCtx->clientId,
                                             0,
                                             static_cast<ITfContextOwnerCompositionSink*>(handler.get()),
                                             &inputCtx->ctx,
                                             &ec));

    source = inputCtx->ctx;
    CHECK_HR(source->AdviseSink(IID_ITfTextEditSink, static_cast<ITfTextEditSink*>(handler.get()), &cookieEditSink));

    COM_HR_END();
    COM_HR_THR();
}

CompositionImpl::~CompositionImpl()
{
    if (cookieEleSink != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfUIElementMgr> eleMgr(IID_ITfUIElementMgr, inputCtx->threadMgr);
        ComQIPtr<ITfSource>       source(IID_ITfSource, eleMgr);
        source->UnadviseSink(cookieEleSink);
        cookieEleSink = TF_INVALID_COOKIE;
    }

    if (cookieEditSink != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, inputCtx->ctx);
        source->UnadviseSink(cookieEditSink);
        cookieEditSink = TF_INVALID_COOKIE;
    }
}

void CompositionImpl::setPreEditRect(const PreEditRect& rect)
{
    this->rect = rect;
}

PreEditRect CompositionImpl::getPreEditRect()
{
    return this->rect;
}

void CompositionImpl::terminate()
{
    COM_HR_BEGIN(S_OK);

    CHECK_HR(inputCtx->assertCreatorThread());

    ComQIPtr<ITfContextOwnerCompositionServices> services(IID_ITfContextOwnerCompositionServices, inputCtx->ctx);
    // Pass Null to terminate all the composition
    services->TerminateComposition(NULL);

    COM_HR_END();
    COM_HR_THR();
}

} // namespace IngameIME::tf
