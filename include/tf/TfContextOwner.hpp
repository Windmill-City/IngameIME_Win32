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

class ContextOwner
    : protected ComObjectBase
    , public ITfContextOwner
{
  protected:
    InputContextImpl* inputCtx;
    DWORD             cookieOwner{TF_INVALID_COOKIE};

  public:
    ContextOwner(InputContextImpl* inputCtx);
    ~ContextOwner();

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
} // namespace IngameIME::tf