#include "tf/TfInputContextImpl.hpp"

namespace IngameIME::tf
{
InputModeHandler::InputModeHandler(InputContextImpl* inputCtx)
    : inputCtx(inputCtx)
{
    COM_HR_BEGIN(S_OK);

    ComPtr<ITfThreadMgr> threadMgr;
    CHECK_HR(getThreadMgr(&threadMgr));
    compMgr = threadMgr;

    CHECK_HR(compMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &mode));

    COM_HR_END();
    COM_HR_THR();
}

HRESULT STDMETHODCALLTYPE InputModeHandler::OnChange(REFGUID rguid)
{
    COM_HR_BEGIN(S_OK);

    if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
    {
        VARIANT var;
        mode->GetValue(&var);

        if (var.intVal & TF_CONVERSIONMODE_NATIVE)
        {
            inputMode = InputMode::Native;
        }
        else
            inputMode = InputMode::AlphaNumeric;

        inputCtx->InputModeCallbackHolder::runCallback(inputMode);
    }

    COM_HR_END();
    COM_HR_RET();
}
} // namespace IngameIME::tf