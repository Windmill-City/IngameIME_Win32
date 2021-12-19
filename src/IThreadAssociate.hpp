#pragma once
#include <processthreadsapi.h>
#include <winerror.h>
namespace libtf {
    class IThreadAssociate {
        DWORD m_CreatorThreadId;

      protected:
        /**
         * @brief Initialize the creator thread
         *
         * @return DWORD creator thread id
         */
        DWORD initialCreatorThread()
        {
            return m_CreatorThreadId = GetCurrentThreadId();
        }

      public:
        /**
         * @brief Assert the calling thread is the thread that create this context
         *
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
         */
        HRESULT assertCreatorThread() const
        {
            if (GetCurrentThreadId() != m_CreatorThreadId) return UI_E_WRONG_THREAD;
            return S_OK;
        }
    };
}// namespace libtf
