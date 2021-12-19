#pragma once
#include <functional>

namespace libtf {
    template <typename... Args>
    class ICallback {
        std::function<void(Args&&...)> m_Callback;

      public:
        std::function<void(Args&&...)> setCallback(std::function<void(Args&&...)> callback)
        {
            auto prev  = m_Callback;
            m_Callback = callback;
            return prev;
        }

        void runCallback(Args&&... args)
        {
            if (m_Callback) m_Callback(std::forward<Args&&>(args)...);
        }
    };
}// namespace libtf