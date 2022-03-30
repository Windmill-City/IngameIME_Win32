#pragma once
#include "..\common\InputProcessorImpl.hpp"

namespace IngameIME::tf
{
class InputProcessorHandler
    : public ComObjectBase
    , public ITfInputProcessorProfileActivationSink
    , public ITfCompartmentEventSink
{
  public:
    ComQIPtr<ITfCompartmentMgr> compMgr{IID_ITfCompartmentMgr};
    ComPtr<ITfCompartment>      mode;

  public:
    InputProcessorHandler()
    {
        COM_HR_BEGIN(S_OK);

        ComPtr<ITfThreadMgr> threadMgr;
        CHECK_HR(getThreadMgr(&threadMgr));
        compMgr = threadMgr;

        CHECK_HR(compMgr->GetCompartment(GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION, &mode));

        COM_HR_END();
        COM_HR_THR();
    }

  protected:
    InputProcessorContext getCtx()
    {
        InputProcessorContext result;

        auto activeProc = InputProcessorImpl::getActiveInputProcessor();
        result.proc     = activeProc;

        VARIANT var;
        mode->GetValue(&var);

        if (activeProc->type == InputProcessorType::KeyboardLayout)
            result.modes.push_back(L"AlphaNumeric");
        else
        {
            if (var.intVal & TF_CONVERSIONMODE_NATIVE)
            {
                result.modes.push_back(L"Native");

                if (activeProc->isJap)
                    if (var.intVal & TF_CONVERSIONMODE_KATAKANA)
                        result.modes.push_back(L"Katakana");
                    else
                        result.modes.push_back(L"Hiragana");
            }
            else
                result.modes.push_back(L"AlphaNumeric");

            if (var.intVal & TF_CONVERSIONMODE_FULLSHAPE)
                result.modes.push_back(L"FullShape");
            else
                result.modes.push_back(L"HalfShape");
        }

        return result;
    }

  public:
    COM_DEF_BEGIN();
    COM_DEF_INF(ITfInputProcessorProfileActivationSink);
    COM_DEF_INF(ITfCompartmentEventSink);
    COM_DEF_END();

    HRESULT STDMETHODCALLTYPE OnActivated(DWORD    dwProfileType,
                                          LANGID   langid,
                                          REFCLSID clsid,
                                          REFGUID  catid,
                                          REFGUID  guidProfile,
                                          HKL      hkl,
                                          DWORD    dwFlags)
    {
        COM_HR_BEGIN(S_OK);

        // Only notify active inputprocessor
        if (!(dwFlags & TF_IPSINK_FLAG_ACTIVE)) return S_OK;

        TF_INPUTPROCESSORPROFILE profile;
        profile.dwProfileType = dwProfileType;
        profile.langid        = langid;
        profile.clsid         = clsid;
        profile.catid         = catid;
        profile.guidProfile   = guidProfile;
        profile.hkl           = hkl;

        Global::getInstance().runCallback(InputProcessorState::FullUpdate, getCtx());

        COM_HR_END();
        COM_HR_RET();
    }

    HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override
    {
        COM_HR_BEGIN(S_OK);

        if (IsEqualGUID(rguid, GUID_COMPARTMENT_KEYBOARD_INPUTMODE_CONVERSION))
            Global::getInstance().runCallback(InputProcessorState::InputModeUpdate, getCtx());

        COM_HR_END();
        COM_HR_RET();
    }
};
} // namespace IngameIME::tf