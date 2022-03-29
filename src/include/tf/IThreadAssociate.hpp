#pragma once
#include <windows.h>
#include <winerror.h>

namespace IngameIME::tf {
    class IThreadAssociate {
      private:
        DWORD threadId;

      protected:
        /**
         * @brief Initialize the creator thread
         *
         * @param threadId creator thread id, set to null to use current thread
         * @return DWORD creator thread id
         */
        DWORD initialCreatorThread(const DWORD threadId = 0)
        {
            return this->threadId = (threadId != 0 ? threadId : GetCurrentThreadId());
        }

      public:
        /**
         * @brief Assert the calling thread is the thread that create this object
         *
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the object
         */
        HRESULT assertCreatorThread() const
        {
            if (GetCurrentThreadId() != threadId) return UI_E_WRONG_THREAD;
            return S_OK;
        }

        /**
         * @brief Get the Creator Thread Id
         *
         * @return DWORD creator thread id
         */
        DWORD getCreatorThreadId() const
        {
            return threadId;
        }
    };
}// namespace IngameIME::tf
