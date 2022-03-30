#include "tf/ComObjectBase.hpp"

namespace IngameIME::tf
{
ULONG ComObjectBase::InternalAddRef(void)
{
    return ++ref;
}

ULONG ComObjectBase::InternalRelease(void)
{
    if (--ref == 0) delete this;
    return ref;
}
} // namespace IngameIME::tf
