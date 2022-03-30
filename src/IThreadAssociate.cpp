#include "tf/IThreadAssociate.hpp"

namespace IngameIME::tf

{

DWORD IThreadAssociate::initialCreatorThread(const DWORD threadId)
{
    return this->threadId = (threadId != 0 ? threadId : GetCurrentThreadId());
}

HRESULT IThreadAssociate::assertCreatorThread() const
{
    if (GetCurrentThreadId() != threadId) return UI_E_WRONG_THREAD;
    return S_OK;
}

DWORD IThreadAssociate::getCreatorThreadId() const
{
    return threadId;
}

} // namespace IngameIME::tf
