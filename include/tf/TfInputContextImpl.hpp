#pragma once
#include <map>

#include "common/InputContextImpl.hpp"
#include "InputProcessor.hpp"

#include <msctf.h>

#include "ComObjectBase.hpp"
#include "ComPtr.hpp"
#include "IThreadAssociate.hpp"
#include "Singleton.hpp"
#include "TfFunction.hpp"

namespace IngameIME::tf
{
class InputContextImpl
    : public InputContext
    , public IThreadAssociate
{
    class ContextOwner;
    friend class CompositionImpl;

  private:
    const HWND hWnd;

    ComPtr<ITfThreadMgr>   threadMgr;
    ComPtr<ITfDocumentMgr> docMgr;
    ComPtr<ITfDocumentMgr> emptyDocMgr;
    ComPtr<ITfContext>     ctx;
    ComPtr<ContextOwner>   owner;
    TfClientId             clientId{TF_CLIENTID_NULL};
    DWORD                  cookie{TF_INVALID_COOKIE};

    bool activated{false};
    bool fullscreen{false};

  protected:
    class ContextOwner
        : protected ComObjectBase
        , public ITfContextOwner
    {
      protected:
        InputContextImpl* ctx;

      public:
        ContextOwner(InputContextImpl* ctx);

      public:
        COM_DEF_BEGIN();
        COM_DEF_INF(ITfContextOwner);
        COM_DEF_END();

      public:
        /**
         * @brief Our implementation does not calculate a text layout
         *
         * @return HRESULT TS_E_NOLAYOUT
         */
        HRESULT STDMETHODCALLTYPE GetACPFromPoint(const POINT* ptScreen, DWORD dwFlags, LONG* pacp) override;
        /**
         * @brief Input method call this method to get the bounding box, in screen coordinates, of preedit string,
         * and use which to position its candidate window
         */
        HRESULT STDMETHODCALLTYPE GetTextExt(LONG acpStart, LONG acpEnd, RECT* prc, BOOL* pfClipped) override;
        /**
         * @brief Return the bounding box, in screen coordinates, of the display surface of the text stream
         */
        HRESULT STDMETHODCALLTYPE GetScreenExt(RECT* prc) override;
        /**
         * @brief Obtains the status of the context.
         */
        HRESULT STDMETHODCALLTYPE GetStatus(TF_STATUS* pdcs) override;
        /**
         * @brief Return the window handle the InputContext is associated to
         *
         * Retrive this value by calling ITfContextView::GetWnd, ITfContextView can query from ITfContext
         */
        HRESULT STDMETHODCALLTYPE GetWnd(HWND* phwnd) override;
        /**
         * @brief Our implementation doesn't support any attributes, just return VT_EMPTY
         */
        HRESULT STDMETHODCALLTYPE GetAttribute(REFGUID rguidAttribute, VARIANT* pvarValue) override;
    };

  public:
    InputContextImpl(const HWND hWnd);
    ~InputContextImpl();

  public:
    /**
     * @brief Set if context activated
     *
     * @param activated set to true to activate input method
     * @throw UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
     */
    virtual void setActivated(const bool activated) override;
    /**
     * @brief Get context activate state
     *
     * @return if the context activated
     */
    virtual bool getActivated() const override;
    /**
     * @brief Set InputContext full screen state
     *
     * @param fullscreen if InputContext full screen
     */
    virtual void setFullScreen(const bool fullscreen) override;
    /**
     * @brief Get if InputContext in full screen state
     *
     * @return true full screen mode
     * @return false window mode
     */
    virtual bool getFullScreen() const override;
};
} // namespace IngameIME::tf