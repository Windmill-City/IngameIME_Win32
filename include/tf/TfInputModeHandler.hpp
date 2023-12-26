#pragma once

#include "common/InputContextImpl.hpp"

#include <map>
#include <msctf.h>

#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "TfFunction.hpp"

namespace IngameIME::tf
{
class InputContextImpl;

class InputModeHandler
    : public ComObjectBase
    , public ITfCompartmentEventSink
{
  private:
    InputContextImpl*           inputCtx;
    ComQIPtr<ITfCompartmentMgr> compMgr{IID_ITfCompartmentMgr};
    ComPtr<ITfCompartment>      mode;
    DWORD                       cookieMode{TF_INVALID_COOKIE};

  public:
    InputMode inputMode = InputMode::AlphaNumeric;

  public:
    InputModeHandler(InputContextImpl* inputCtx);
    ~InputModeHandler();

  public:
    COM_DEF_BEGIN();
    COM_DEF_INF(ITfCompartmentEventSink);
    COM_DEF_END();

    HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override;
};
} // namespace IngameIME::tf