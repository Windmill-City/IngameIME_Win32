#include "tf/TfInputModeHandler.hpp"
#include "tf/TfCompositionHandler.hpp"
#include "tf/TfContextOwner.hpp"
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

    ComQIPtr<ITfSource> source(IID_ITfSource, mode);
    CHECK_HR(source->AdviseSink(IID_ITfCompartmentEventSink, static_cast<ITfCompartmentEventSink*>(this), &cookieMode));

    COM_HR_END();
    COM_HR_THR();
}

InputModeHandler::~InputModeHandler()
{
    if (cookieMode != TF_INVALID_COOKIE)
    {
        ComQIPtr<ITfSource> source(IID_ITfSource, mode);
        source->UnadviseSink(cookieMode);
        cookieMode = TF_INVALID_COOKIE;
    }
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