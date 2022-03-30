#pragma once
#include "common/InputProcessorImpl.hpp"
#include "IngameIME.hpp"

#include "ImmInputContextImpl.hpp"

namespace IngameIME::imm
{
class GlobalImpl : public Global
{
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
     * @return std::shared_ptr<InputContext>
     */
    virtual std::shared_ptr<InputContext>                    getInputContext(void* hWnd, ...) override;
};
} // namespace IngameIME::imm