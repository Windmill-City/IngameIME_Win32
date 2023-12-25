class InputModeHandler
    : public ComObjectBase
    , public ITfCompartmentEventSink
{
  private:
    InputContextImpl*           inputCtx;
    ComQIPtr<ITfCompartmentMgr> compMgr{IID_ITfCompartmentMgr};
    ComPtr<ITfCompartment>      mode;

  public:
    InputMode inputMode = InputMode::AlphaNumeric;

  public:
    InputModeHandler(InputContextImpl* inputCtx);

  public:
    COM_DEF_BEGIN();
    COM_DEF_INF(ITfCompartmentEventSink);
    COM_DEF_END();

    HRESULT STDMETHODCALLTYPE OnChange(REFGUID rguid) override;
};
