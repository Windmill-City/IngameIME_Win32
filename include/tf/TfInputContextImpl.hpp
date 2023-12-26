#pragma once
#include "common/InputContextImpl.hpp"

#include <map>
#include <msctf.h>

#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "TfFunction.hpp"

namespace IngameIME::tf
{
class ContextOwner;
class CompositionHandler;
class InputModeHandler;

class InputContextImpl : public InputContext
{
  private:
    friend class ContextOwner;
    friend class CompositionHandler;
    friend class InputModeHandler;

    const HWND hWnd;

    ComPtr<ITfThreadMgr>   threadMgr;
    ComPtr<ITfDocumentMgr> docMgr;
    ComPtr<ITfDocumentMgr> emptyDocMgr;
    ComPtr<ITfContext>     ctx;
    TfClientId             clientId{TF_CLIENTID_NULL};

    ComPtr<ContextOwner>       owner;
    ComPtr<CompositionHandler> h_comp;
    ComPtr<InputModeHandler>   h_mode;

    PreEditRect rect;

    bool activated{false};
    bool fullscreen{false};

  public:
    InputContextImpl(const HWND hWnd);
    ~InputContextImpl();

  public:
    virtual InputMode   getInputMode() override;
    virtual void        setPreEditRect(const PreEditRect& rect) override;
    virtual PreEditRect getPreEditRect() override;
    virtual void        setActivated(const bool activated) override;
    virtual bool        getActivated() const override;
    virtual void        setFullScreen(const bool fullscreen) override;
    virtual bool        getFullScreen() const override;
};
} // namespace IngameIME::tf