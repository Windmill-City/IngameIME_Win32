#pragma once
#include <string>

#include "ComBSTR.hpp"
#include "TfInputContextImpl.hpp"

namespace IngameIME::tf
{
class CompositionImpl : public Composition
{
  private:
    InternalRect rect;
    DWORD cookieEditSink{TF_INVALID_COOKIE};
    DWORD cookieEleSink{TF_INVALID_COOKIE};

  protected:
    class CompositionHandler;
    InputContextImpl*          inputCtx;
    ComPtr<CompositionHandler> handler;

  protected:
    class CompositionHandler
        : protected ComObjectBase
        , public ITfContextOwnerCompositionSink
        , public ITfTextEditSink
        , public ITfEditSession
        , public ITfUIElementSink
    {
      protected:
        CompositionImpl* comp;

        ComPtr<ITfCompositionView> compView;

        ComQIPtr<ITfUIElementMgr>         eleMgr{IID_ITfUIElementMgr};
        ComPtr<ITfCandidateListUIElement> ele;
        DWORD                             eleId{TF_INVALID_UIELEMENTID};

      public:
        CompositionHandler(CompositionImpl* comp);

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
         *
         * @note Selection change only triggers OnEndEdit event,
         *  for convenient, we handle preedit here at the same time
         */
        HRESULT STDMETHODCALLTYPE OnEndEdit(ITfContext* pic, TfEditCookie ec, ITfEditRecord* pEditRecord) override;
        /**
         * @brief Get all the text in the context, which is commit string
         */
        HRESULT STDMETHODCALLTYPE DoEditSession(TfEditCookie ec) override;

      public:
        /**
         * @brief Hide all the window of the input method if in full screen mode
         */
        HRESULT STDMETHODCALLTYPE BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override;
        HRESULT STDMETHODCALLTYPE UpdateUIElement(DWORD dwUIElementId) override;
        HRESULT STDMETHODCALLTYPE EndUIElement(DWORD dwUIElementId) override;
    };

  public:
    CompositionImpl(InputContextImpl* inputCtx);
    ~CompositionImpl();

  public:
    /**
     * @brief Bounding box of the PreEdit text drawn by the Application, the position is in window coordinate
     *
     */
    virtual void        setPreEditRect(const PreEditRect& rect) override;
    /**
     * @brief Bounding box of the PreEdit text drawn by the Application, the position is in window coordinate
     *
     */
    virtual PreEditRect getPreEditRect() override;
    /**
     * @brief Terminate active composition
     */
    virtual void terminate() override;
};
} // namespace IngameIME::tf
