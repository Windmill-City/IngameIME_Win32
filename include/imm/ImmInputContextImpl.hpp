#pragma once
#include <map>

#include <windows.h>
#pragma comment(lib, "imm32.lib")

#include "IngameIME.hpp"

#include "common/InputContextImpl.hpp"
#include "common/InputProcessorImpl.hpp"
#include "Singleton.hpp"

namespace IngameIME::imm
{
class InputContextImpl
    : public InputContext
    , public Singleton<InputContextImpl, HWND>
{
  protected:
    static LRESULT WndProc(HWND hWnd, UINT msg, WPARAM wparam, LPARAM lparam);

  protected:
    HWND    hWnd;
    WNDPROC prevProc;

    HIMC ctx;

    bool activated{false};
    bool fullscreen{false};

    friend class CompositionImpl;
    friend class GlobalImpl;

  public:
    InputContextImpl(const HWND hWnd);
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
    InputProcessorContext getInputProcCtx();

  public:
    /**
     * @brief Set InputContext activate state
     *
     * @param activated if InputContext activated
     */
    virtual void setActivated(const bool activated) noexcept override;
    /**
     * @brief Get if InputContext activated
     *
     * @return true activated
     * @return false not activated
     */
    virtual bool getActivated() const noexcept override;
    /**
     * @brief Set InputContext full screen state
     *
     * @param fullscreen if InputContext full screen
     */
    virtual void setFullScreen(const bool fullscreen) noexcept override;
    /**
     * @brief Get if InputContext in full screen state
     *
     * @return true full screen mode
     * @return false window mode
     */
    virtual bool getFullScreen() const noexcept override;
};
} // namespace IngameIME::imm