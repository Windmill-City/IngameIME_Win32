/**
Copyright (c) 2012 Jakob Progsch, Václav Zeman

This software is provided 'as-is', without any express or implied
warranty. In no event will the authors be held liable for any damages
arising from the use of this software.

Permission is granted to anyone to use this software for any purpose,
including commercial applications, and to alter it and redistribute it
freely, subject to the following restrictions:

   1. The origin of this software must not be misrepresented; you must not
   claim that you wrote the original software. If you use this software
   in a product, an acknowledgment in the product documentation would be
   appreciated but is not required.

   2. Altered source versions must be plainly marked as such, and must not be
   misrepresented as being the original software.

   3. This notice may not be removed or altered from any source
   distribution.
*/

#pragma once
#include "libtfdef.h"
#include <combaseapi.h>
#include <condition_variable>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace libtf
{
    /**
     * @brief STA Apartment Thread
     */
    class TfThread
    {
    private:
        std::thread m_tfThread;
        DWORD m_tfThreadId;
        std::mutex m_queue_mutex;
        std::queue<std::function<void()>> m_tasks;

    public:
        TfThread()
        {
            std::promise<bool> messageQueueReady;
            auto future = messageQueueReady.get_future();
            m_tfThread = std::thread(
                [this](std::promise<bool> messageQueueReady)
                {
                    m_tfThreadId = GetCurrentThreadId();
                    CoInitialize(NULL);

                    MSG msg;
                    PeekMessage(&msg, NULL, WM_USER, WM_USER, PM_NOREMOVE);
                    messageQueueReady.set_value(true);

                    while (GetMessage(&msg, NULL, 0, 0))
                    {
                        TranslateMessage(&msg);
                        DispatchMessage(&msg);

                        std::function<void()> task;
                        {
                            std::unique_lock<std::mutex> lock(m_queue_mutex);
                            if (m_tasks.empty()) continue;
                            task = std::move(this->m_tasks.front());
                            this->m_tasks.pop();
                        }
                        task();
                    }

                    CoUninitialize();
                }, std::move(messageQueueReady));
            m_tfThread.detach();
            future.get();
        }

        /**
         * @brief Get thread id
         * 
         * @return DWORD
         */
        DWORD getId()
        {
            return m_tfThreadId;
        }

        /**
         * @brief Async run task on this thread
         */
        template <class F, class... Args>
        auto enqueue(F &&f, Args &&...args)
            -> std::future<typename std::result_of<F(Args...)>::type>
        {
            using return_type = typename std::result_of<F(Args...)>::type;

            auto task = std::make_shared<std::packaged_task<return_type()>>(
                std::bind(std::forward<F>(f), std::forward<Args>(args)...));

            std::future<return_type> res = task->get_future();
            {
                std::unique_lock<std::mutex> lock(m_queue_mutex);

                m_tasks.emplace([task]()
                                { (*task)(); });
            }
            //Notify
            PostThreadMessage(getId(), WM_NULL, NULL, NULL);
            return res;
        }
    };
}