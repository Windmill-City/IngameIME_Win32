#pragma once
#include "common/InputProcessorImpl.hpp"

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
    InputProcessorHandler();

  protected:
    InputProcessorContext getCtx();

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
                                          DWORD    dwFlags) override;

    HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override;
};
} // namespace IngameIME::tf