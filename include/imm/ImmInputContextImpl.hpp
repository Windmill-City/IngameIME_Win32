#pragma once

#include <list>

#include <windows.h>
#pragma comment(lib, "imm32.lib")

#include "common/InputContextImpl.hpp"

namespace IngameIME::imm
{
class InputContextImpl : public InputContext
{
  protected:
    static std::list<InputContextImpl*> ActiveContexts;
    static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

  protected:
    HWND    hWnd;
    WNDPROC prevProc;

    HIMC ctx;

    PreEditRect rect;

    bool activated = false;
    bool uiLess    = false;

  public:
    InputContextImpl(const HWND hWnd, const bool uiLess);
    ~InputContextImpl();

  protected:
    /**
     * @brief Retrive PreEdit info for current Composition
     */
    void procPreEdit();
    /**
     * @brief Retrive Commit text for current Composition
     */
    void procCommit();
    /**
     * @brief Set CandidateList window's position for current Composition
     */
    void setPreEditRect(InternalRect& rect);
    /**
     * @brief Retrive CandidateList infomation for application to draw
     */
    void procCand();

  public:
    virtual InputMode   getInputMode() override;
    virtual void        setPreEditRect(const PreEditRect& rect) override;
    virtual PreEditRect getPreEditRect() override;
    virtual void        setActivated(const bool activated) override;
    virtual bool        getActivated() const override;
};
} // namespace IngameIME::imm