#include "imm\ImmInputContextImpl.hpp"

#include "imm\ImmCompositionImpl.hpp"

IngameIME::imm::InputContextImpl::Singleton::RefHolderType IngameIME::imm::InputContextImpl::WeakRefs = {};
std::mutex IngameIME::imm::InputContextImpl::RefHolderMutex                                           = std::mutex();

namespace IngameIME::imm
{
LRESULT InputContextImpl::WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam)
{
    auto iter = WeakRefs.find(hWnd);

    std::shared_ptr<InputContextImpl> inputCtx;
    if (iter != WeakRefs.end() && (inputCtx = (*iter).second.lock()))
    {
        switch (msg)
        {
        case WM_INPUTLANGCHANGE:
            Global::getInstance().runCallback(InputProcessorState::FullUpdate, inputCtx->getInputProcCtx());
            break;
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
            inputCtx->comp->PreEditCallbackHolder::runCallback(CompositionState::Begin, nullptr);
            return true;
        case WM_IME_COMPOSITION:
            if (lparam & (GCS_COMPSTR | GCS_CURSORPOS)) inputCtx->procPreEdit();
            if (lparam & GCS_RESULTSTR) inputCtx->procCommit();

            if (!inputCtx->fullscreen) inputCtx->procPreEditRect();

            // when lparam == 0 that means current Composition has been
            // canceled
            if (lparam) return true;
        case WM_IME_ENDCOMPOSITION:
            inputCtx->comp->PreEditCallbackHolder::runCallback(CompositionState::End, nullptr);
            inputCtx->comp->CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);
            return true;
        case WM_IME_NOTIFY:
            if (inputCtx->fullscreen) switch (wparam)
                {
                case IMN_OPENCANDIDATE:
                    inputCtx->comp->CandidateListCallbackHolder::runCallback(CandidateListState::Begin, nullptr);
                    return true;
                case IMN_CHANGECANDIDATE:
                    inputCtx->procCand();
                    return true;
                case IMN_CLOSECANDIDATE:
                    inputCtx->comp->CandidateListCallbackHolder::runCallback(CandidateListState::End, nullptr);
                    return true;
                default:
                    break;
                }
            if (wparam == IMN_SETCONVERSIONMODE)
            {
                Global::getInstance().runCallback(InputProcessorState::InputModeUpdate, inputCtx->getInputProcCtx());
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
    : Singleton(hWnd)
    , hWnd(hWnd)
{
    comp = std::make_shared<CompositionImpl>(this);

    // Reset to default context
    ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
    ctx = ImmAssociateContext(hWnd, NULL);

    prevProc = (WNDPROC)SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)InputContextImpl::WndProc);
}

InputContextImpl::~InputContextImpl()
{
    comp->terminate();
    ImmAssociateContextEx(hWnd, NULL, IACE_DEFAULT);
    SetWindowLongPtr(hWnd, GWLP_WNDPROC, (LONG_PTR)prevProc);
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
    ctx.content  = std::wstring(buf.get(), size / sizeof(WCHAR));
    ctx.selStart = ctx.selEnd = sel;

    comp->PreEditCallbackHolder::runCallback(CompositionState::Update, &ctx);
}

void InputContextImpl::procCommit()
{
    auto size = ImmGetCompositionStringW(ctx, GCS_RESULTSTR, NULL, 0);
    // Error occurs
    if (size <= 0) return;

    auto buf = std::make_unique<WCHAR[]>(size / sizeof(WCHAR));
    ImmGetCompositionStringW(ctx, GCS_RESULTSTR, buf.get(), size);

    comp->CommitCallbackHolder::runCallback(std::wstring(buf.get(), size / sizeof(WCHAR)));
}

void InputContextImpl::procPreEditRect()
{
    InternalRect rect;
    comp->PreEditRectCallbackHolder::runCallback(rect);

    CANDIDATEFORM cand;
    cand.dwIndex        = 0;
    cand.dwStyle        = CFS_EXCLUDE;
    cand.ptCurrentPos.x = rect.left;
    cand.ptCurrentPos.y = rect.top;
    cand.rcArea         = rect;
    ImmSetCandidateWindow(ctx, &cand);

    COMPOSITIONFORM comp;
    comp.dwStyle        = CFS_RECT;
    comp.ptCurrentPos.x = rect.left;
    comp.ptCurrentPos.y = rect.top;
    comp.rcArea         = rect;
    ImmSetCompositionWindow(ctx, &comp);
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

        candCtx.candidates.push_back(std::wstring((wchar_t*)strStart, len));
    }

    comp->CandidateListCallbackHolder::runCallback(CandidateListState::Update, &candCtx);
}

InputProcessorContext InputContextImpl::getInputProcCtx()
{
    InputProcessorContext result;

    DWORD mode;
    ImmGetConversionStatus(ctx, &mode, NULL);

    auto activeProc = InputProcessorImpl::getActiveInputProcessor();

    std::list<InputMode> modes;
    if (activeProc->type == InputProcessorType::KeyboardLayout)
        modes.push_back(InputMode::AlphaNumeric);
    else
    {
        if (mode & IME_CMODE_NATIVE)
        {
            modes.push_back(InputMode::Native);

            if (activeProc->isJap)
                if (mode & IME_CMODE_KATAKANA)
                    modes.push_back(InputMode::Katakana);
                else
                    modes.push_back(InputMode::Hiragana);
        }
        else
            modes.push_back(InputMode::AlphaNumeric);

        if (mode & IME_CMODE_FULLSHAPE)
            modes.push_back(InputMode::FullShape);
        else
            modes.push_back(InputMode::HalfShape);
    }

    result.proc  = activeProc;
    result.modes = modes;

    return result;
}

void InputContextImpl::setActivated(const bool activated) noexcept
{
    this->activated = activated;

    if (activated)
        ImmAssociateContext(hWnd, ctx);
    else
        ImmAssociateContext(hWnd, NULL);
}

bool InputContextImpl::getActivated() const noexcept
{
    return activated;
}

void InputContextImpl::setFullScreen(const bool fullscreen) noexcept
{
    this->fullscreen = fullscreen;

    if (activated)
    {
        comp->terminate();
        // Refresh InputContext
        ImmAssociateContext(hWnd, NULL);
        ImmAssociateContext(hWnd, ctx);
    }
}

bool InputContextImpl::getFullScreen() const noexcept
{
    return fullscreen;
}

} // namespace IngameIME::imm
