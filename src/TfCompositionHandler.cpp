#include "tf/TfInputModeHandler.hpp"
#include "tf/TfCompositionHandler.hpp"
#include "tf/TfContextOwner.hpp"
#include "tf/TfInputContextImpl.hpp"

#include "tf/ComBSTR.hpp"

namespace IngameIME::tf
{

CompositionHandler::CompositionHandler(InputContextImpl* inputCtx)
    : inputCtx(inputCtx)
{
    eleMgr = inputCtx->threadMgr;
}

HRESULT STDMETHODCALLTYPE CompositionHandler::OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk)
{
    if (!pfOk) return E_INVALIDARG;
    *pfOk = true; // allow composition start

    inputCtx->PreEditCallbackHolder::runCallback(CompositionState::Begin, nullptr);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompositionHandler::OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew)
{
    // Process in ITfTextEditSink
    compView = pComposition;
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompositionHandler::OnEndComposition(ITfCompositionView* pComposition)
{
    compView.reset();
    inputCtx->PreEditCallbackHolder::runCallback(CompositionState::End, nullptr);

    static HRESULT hr;
    hr = inputCtx->ctx->RequestEditSession(inputCtx->clientId, this, TF_ES_ASYNC | TF_ES_READWRITE, &hr);
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CompositionHandler::OnEndEdit(ITfContext* pic, TfEditCookie ec, ITfEditRecord* pEditRecord)
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
    CHECK_HR(inputCtx->ctx->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &fetched));
    selRange.attach(sel[0].range);
    rangeAcp = selRange;
    CHECK_HR(rangeAcp->GetExtent(&acpStart, &len));

    PreEditContext preEditCtx;
    preEditCtx.selStart = acpStart;
    preEditCtx.selEnd   = acpStart + len;
    preEditCtx.content  = ToUTF8(std::wstring(bufPreEdit.get(), preEditLen));

    inputCtx->PreEditCallbackHolder::runCallback(CompositionState::Update, &preEditCtx);

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionHandler::DoEditSession(TfEditCookie ec)
{
    COM_HR_BEGIN(S_OK);

    auto ctx = inputCtx->ctx;

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

    inputCtx->CommitCallbackHolder::runCallback(ToUTF8(std::wstring(bufCommit.get(), commitLen)));

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionHandler::BeginUIElement(DWORD dwUIElementId, BOOL* pbShow)
{
    COM_HR_BEGIN(S_OK);

    if (dwUIElementId == TF_INVALID_UIELEMENTID) return E_INVALIDARG;
    if (!pbShow) return E_INVALIDARG;

    // Set if CandidateWindow should display
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
        inputCtx->CandidateListCallbackHolder::runCallback(CandidateListState::Begin, nullptr);
    }

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionHandler::UpdateUIElement(DWORD dwUIElementId)
{
    COM_HR_BEGIN(S_OK);

    if (dwUIElementId == TF_INVALID_UIELEMENTID) return E_INVALIDARG;
    if (eleId == TF_INVALID_UIELEMENTID || dwUIElementId != eleId) return S_OK;

    // Total count of Candidates
    uint32_t totalCount;
    CHECK_HR(ele->GetCount(&totalCount));

    // How many pages
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

    // Absolute index of current selected candidate
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
            candCtx.candidates.push_back("[err]");
        else
            candCtx.candidates.push_back(ToUTF8(candidate.bstr));
    }

    inputCtx->CandidateListCallbackHolder::runCallback(CandidateListState::Update, &candCtx);

    COM_HR_END();
    COM_HR_RET();
}

HRESULT STDMETHODCALLTYPE CompositionHandler::EndUIElement(DWORD dwUIElementId)
{
    COM_HR_BEGIN(S_OK);

    if (dwUIElementId == TF_INVALID_UIELEMENTID) return E_INVALIDARG;
    if (eleId == TF_INVALID_UIELEMENTID || dwUIElementId != eleId) return S_OK;

    eleId = TF_INVALID_UIELEMENTID;
    ele.reset();

    inputCtx->CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);

    COM_HR_END();
    COM_HR_RET();
}
} // namespace IngameIME::tf