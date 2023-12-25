class CompositionHandler
    : protected ComObjectBase
    , public ITfContextOwnerCompositionSink
    , public ITfTextEditSink
    , public ITfEditSession
    , public ITfUIElementSink
{
  protected:
    InputContextImpl* inputCtx;

    ComPtr<ITfCompositionView> compView;

    ComQIPtr<ITfUIElementMgr>         eleMgr{IID_ITfUIElementMgr};
    ComPtr<ITfCandidateListUIElement> ele;
    DWORD                             eleId{TF_INVALID_UIELEMENTID};

  public:
    CompositionHandler(InputContextImpl* inputCtx);

  public:
    COM_DEF_BEGIN();
    COM_DEF_INF(ITfContextOwnerCompositionSink);
    COM_DEF_INF(ITfTextEditSink);
    COM_DEF_INF(ITfEditSession);
    COM_DEF_INF(ITfUIElementSink);
    COM_DEF_END();

  public:
    HRESULT STDMETHODCALLTYPE OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override;
    HRESULT STDMETHODCALLTYPE OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override;
    HRESULT STDMETHODCALLTYPE OnEndComposition(ITfCompositionView* pComposition) override;

  public:
    /**
     * @brief Get PreEdit text and its selection
     */
    HRESULT STDMETHODCALLTYPE OnEndEdit(ITfContext* pic, TfEditCookie ec, ITfEditRecord* pEditRecord) override;
    /**
     * @brief Get converted text
     */
    HRESULT STDMETHODCALLTYPE DoEditSession(TfEditCookie ec) override;

  public:
    /**
     * @brief Hide CandidateList Window of the input method in full screen mode
     */
    HRESULT STDMETHODCALLTYPE BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override;
    HRESULT STDMETHODCALLTYPE UpdateUIElement(DWORD dwUIElementId) override;
    HRESULT STDMETHODCALLTYPE EndUIElement(DWORD dwUIElementId) override;
};