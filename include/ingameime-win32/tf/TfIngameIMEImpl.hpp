#pragma once
#include "IngameIME.hpp"

#include "TfInputContextImpl.hpp"
#include "TfInputProcessorImpl.hpp"

namespace IngameIME::tf
{
class GlobalImpl : public Global
{
  private:
    DWORD cookieComp{TF_INVALID_COOKIE};
    DWORD cookieProc{TF_INVALID_COOKIE};

  protected:
    ComPtr<InputProcessorHandler> handler;
    friend class Global;

  protected:
    GlobalImpl();
    ~GlobalImpl();

  public:
    /**
     * @brief Get Active InputProcessor
     *
     * @return std::shared_ptr<InputProcessor>
     */
    virtual std::shared_ptr<const InputProcessor>            getActiveInputProcessor() const override;
    /**
     * @brief Get system availiable InputProcessor
     *
     * @return std::list<std::shared_ptr<InputProcessor>>
     */
    virtual std::list<std::shared_ptr<const InputProcessor>> getInputProcessors() const override;
    /**
     * @brief Get the InputContext object
     *
     * @param hWnd the window to create InputContext
     * @return std::shared_ptr<InputContext>
     */
    virtual std::shared_ptr<InputContext>                    getInputContext(void* hWnd, ...) override;
};
} // namespace IngameIME::tf