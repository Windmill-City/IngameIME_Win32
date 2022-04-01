#include <stdarg.h>

#include "imm\ImmIngameIMEImpl.hpp"
#include "tf\TfIngameIMEImpl.hpp"

#include <versionhelpers.h>

IngameIME::Global& IngameIME::Global::getInstance(void* ignore, ...)
{
    thread_local IngameIME::Global& Instance =
        IsWindows8OrGreater() ? (IngameIME::Global&)*new tf::GlobalImpl() : (IngameIME::Global&)*new imm::GlobalImpl();
    return Instance;
}