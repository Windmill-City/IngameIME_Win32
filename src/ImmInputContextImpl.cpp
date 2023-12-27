#include "imm\ImmInputContextImpl.hpp"

#include <algorithm>

namespace IngameIME::imm
{
std::list<InputContextImpl*> InputContextImpl::ActiveContexts = {};

LRESULT InputContextImpl::WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    auto iter = std::find_if(ActiveContexts.begin(),
                             ActiveContexts.end(),
                             [hWnd](InputContextImpl* it) { return it->hWnd == hWnd; });

    InputContextImpl* inputCtx;
    if (iter != ActiveContexts.end() && (inputCtx = *iter))
    {
        switch (msg)
        {
        case WM_IME_SETCONTEXT:
            // We should always hide Composition Window to make the
            // PreEditCallback work
            lparam &= ~ISC_SHOWUICOMPOSITIONWINDOW;

            if (inputCtx->fullscreen)
            {
                // Hide Candidate Window
                lparam &= ~ISC_SHOWUICANDIDATEWINDOW;
            }
            break;
        case WM_IME_STARTCOMPOSITION:
            inputCtx->PreEditCallbackHolder::runCallback(CompositionState::Begin, nullptr);
            return true;
        case WM_IME_COMPOSITION:
            if (lparam & (GCS_COMPSTR | GCS_CURSORPOS)) inputCtx->procPreEdit();
            if (lparam & GCS_RESULTSTR) inputCtx->procCommit();
            return true;
        case WM_IME_ENDCOMPOSITION:
            inputCtx->PreEditCallbackHolder::runCallback(CompositionState::End, nullptr);
            inputCtx->CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);
            return true;
        case WM_IME_NOTIFY:
            switch (wparam)
            {
            case IMN_OPENCANDIDATE:
                if (inputCtx->fullscreen)
                    inputCtx->CandidateListCallbackHolder::runCallback(CandidateListState::Begin, nullptr);
                return true;
            case IMN_CHANGECANDIDATE:
                if (inputCtx->fullscreen) inputCtx->procCand();
                return true;
            case IMN_CLOSECANDIDATE:
                if (inputCtx->fullscreen)
                    inputCtx->CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);
                return true;
            default:
                break;
            }
            if (wparam == IMN_SETCONVERSIONMODE)
            {
                inputCtx->InputModeCallbackHolder::runCallback(inputCtx->getInputMode());
            }
            break;
        case WM_IME_CHAR:
            // Commit text already handled at WM_IME_COMPOSITION
            return true;
        default:
            return CallWindowProcW(inputCtx->prevProc, hWnd, msg, wparam, lparam);
        }
    }
    return DefWindowProcW(hWnd, msg, wparam, lparam);
}

IngameIME::imm::InputContextImpl::InputContextImpl(const HWND hWnd)
    : hWnd(hWnd)
{
    // Reset to default context
    ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
    ctx = ImmAssociateContext(hWnd, NULL);

    prevProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)InputContextImpl::WndProc);

    ActiveContexts.emplace_front(this);
}

InputContextImpl::~InputContextImpl()
{
    ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)prevProc);

    ActiveContexts.remove(this);
}

void InputContextImpl::procPreEdit()
{
    // PreEdit Text
    auto size = ImmGetCompositionStringW(ctx, GCS_COMPSTR, NULL, 0);
    // Error occurs
    if (size <= 0) return;

    auto buf = std::make_unique<WCHAR[]>(size / sizeof(WCHAR));
    ImmGetCompositionStringW(ctx, GCS_COMPSTR, buf.get(), size);

    // Selection
    int sel = ImmGetCompositionStringW(ctx, GCS_CURSORPOS, NULL, 0);

    PreEditContext ctx;
    ctx.content  = ToUTF8(std::wstring(buf.get(), size / sizeof(WCHAR)));
    ctx.selStart = ctx.selEnd = sel;

    this->PreEditCallbackHolder::runCallback(CompositionState::Update, &ctx);
}

void InputContextImpl::procCommit()
{
    auto size = ImmGetCompositionStringW(ctx, GCS_RESULTSTR, NULL, 0);
    // Error occurs
    if (size <= 0) return;

    auto buf = std::make_unique<WCHAR[]>(size / sizeof(WCHAR));
    ImmGetCompositionStringW(ctx, GCS_RESULTSTR, buf.get(), size);
    this->CommitCallbackHolder::runCallback(ToUTF8(std::wstring(buf.get(), size / sizeof(WCHAR))));
}

void InputContextImpl::procCand()
{
    CandidateListContext candCtx;

    auto size = ImmGetCandidateListW(ctx, 0, NULL, 0);
    // Error occurs
    if (size == 0) return;

    auto buf  = std::make_unique<byte[]>(size);
    auto cand = (LPCANDIDATELIST)buf.get();

    ImmGetCandidateListW(ctx, 0, cand, size);

    auto pageSize  = cand->dwPageSize;
    auto candCount = cand->dwCount;

    auto pageStart    = cand->dwPageStart;
    auto pageEnd      = pageStart + pageSize;
    candCtx.selection = cand->dwSelection;
    // Absolute index to relative index
    candCtx.selection -= pageStart;

    for (size_t i = 0; i < pageSize; i++)
    {
        auto strStart = buf.get() + cand->dwOffset[i + pageStart];
        auto strEnd   = buf.get() + (((i + pageStart + 1) < candCount) ? cand->dwOffset[i + pageStart + 1] : size);
        auto len      = (strEnd - strStart) / sizeof(WCHAR);

        candCtx.candidates.push_back(ToUTF8(std::wstring((wchar_t*)strStart, len)));
    }
    this->CandidateListCallbackHolder::runCallback(CandidateListState::Update, &candCtx);
}

InputMode InputContextImpl::getInputMode()
{
    DWORD mode;
    ImmGetConversionStatus(ctx, &mode, NULL);

    if (mode & IME_CMODE_NATIVE)
        return InputMode::Native;
    else
        return InputMode::AlphaNumeric;
}

void InputContextImpl::setPreEditRect(const PreEditRect& _rect)
{
    this->rect         = _rect;
    InternalRect  rect = _rect;
    CANDIDATEFORM cand;
    cand.dwIndex        = 0;
    cand.dwStyle        = CFS_EXCLUDE;
    cand.ptCurrentPos.x = rect.x;
    cand.ptCurrentPos.y = rect.y;
    cand.rcArea         = rect;
    ImmSetCandidateWindow(ctx, &cand);

    COMPOSITIONFORM comp;
    comp.dwStyle        = CFS_RECT;
    comp.ptCurrentPos.x = rect.x;
    comp.ptCurrentPos.y = rect.y;
    comp.rcArea         = rect;
    ImmSetCompositionWindow(ctx, &comp);
}

PreEditRect InputContextImpl::getPreEditRect()
{
    return this->rect;
}

void InputContextImpl::setActivated(const bool activated)
{
    if (activated == this->activated) return;
    if (activated)
    {
        ImmAssociateContext(hWnd, ctx);
        InputModeCallbackHolder::runCallback(getInputMode());
    }
    else
    {
        ImmNotifyIME(ctx, NI_COMPOSITIONSTR, CPS_CANCEL, NULL);
        ImmAssociateContext(hWnd, NULL);
        // Some input method does not send message correctly, force end on deactivated
        PreEditCallbackHolder::runCallback(CompositionState::End, nullptr);
        CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);
    }
    this->activated = activated;
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

} // namespace IngameIME::imm
