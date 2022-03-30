#include "imm/ImmIngameIMEImpl.hpp"

namespace IngameIME::imm

{

std::shared_ptr<const InputProcessor> GlobalImpl::getActiveInputProcessor() const
{
    return InputProcessorImpl::getActiveInputProcessor();
}

std::list<std::shared_ptr<const InputProcessor>> GlobalImpl::getInputProcessors() const
{
    return InputProcessorImpl::getInputProcessors();
}

std::shared_ptr<InputContext> GlobalImpl::getInputContext(void* hWnd, ...)
{
    return InputContextImpl::getOrCreate(reinterpret_cast<HWND>(hWnd));
}
} // namespace IngameIME::imm
