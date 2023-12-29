#include "imm/ImmInputContextImpl.hpp"
#include "tf/TfInputContextImpl.hpp"

#include "IngameIME.hpp"

namespace IngameIME
{
InputContext* CreateInputContextWin32(const HWND hWnd, const API api, const bool uiLess)
{
    if (api == API::TextServiceFramework)
        return new tf::InputContextImpl(hWnd, uiLess);
    else
        return new imm::InputContextImpl(hWnd, uiLess);
}
} // namespace IngameIME