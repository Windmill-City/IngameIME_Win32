#include "imm/ImmInputContextImpl.hpp"
#include "tf/TfInputContextImpl.hpp"

#include "IngameIME.hpp"

namespace IngameIME
{
InputContext* CreateInputContextWin32(HWND hWnd, API api)
{
    if (api == API::TextServiceFramework)
        return new tf::InputContextImpl(hWnd);
    else
        return new imm::InputContextImpl(hWnd);
}
} // namespace IngameIME