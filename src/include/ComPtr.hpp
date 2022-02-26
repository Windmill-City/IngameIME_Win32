#pragma once
#include <windows.h>

namespace libtf {
    template <class T>
    class ComPtr {
      protected:
        T* ptr{nullptr};

      public:
        constexpr ComPtr() = default;
        constexpr ComPtr(std::nullptr_t) noexcept {}

        explicit ComPtr(T* ptr) noexcept : ptr(ptr)
        {
            if (ptr) ptr->AddRef();
        }

        ComPtr(const ComPtr& other) noexcept
        {
            ptr = other.ptr;
            if (ptr) ptr->AddRef();
        }

        ComPtr(const ComPtr&& other) noexcept
        {
            ptr       = other.ptr;
            other.ptr = nullptr;
        }

        ~ComPtr() noexcept
        {
            if (ptr) ptr->Release();
        }

      public:
        [[nodiscard]] T* get() const noexcept
        {
            return ptr;
        }

        void swap(ComPtr& other) noexcept
        {
            std::swap(ptr, other.ptr);
        }

        void reset() noexcept
        {
            ComPtr().swap(*this);
        }

        void reset(T* ptr) noexcept
        {
            ComPtr(ptr).swap(*this);
        }

        /**
         * @brief Attach to excting interface without AddRef()
         *
         * @param ptr pointer to interface
         */
        void attach(T* ptr) noexcept
        {
            reset();
            this->ptr = ptr;
        }

        /**
         * @brief Detach the interface without Release()
         *
         * @return pointer to the detached interface
         */
        T* detach() noexcept
        {
            T* p = ptr;
            ptr  = nullptr;
            return p;
        }

        ComPtr& operator=(const ComPtr& right) noexcept
        {
            ComPtr(right).swap(*this);
            return *this;
        }

        ComPtr& operator=(T* ptr) noexcept
        {
            ComPtr(ptr).swap(*this);
            return *this;
        }

        [[nodiscard]] T& operator*() const noexcept
        {
            return *ptr;
        }

        [[nodiscard]] T* operator->() const noexcept
        {
            return ptr;
        }

        /**
         * @brief Acquire address of raw pointer
         *
         * this usually use for acquiring the interface, so the pointer must be null,
         * if you do want to acquire the address of the raw pointer, use &get() instead
         *
         * @return address of the raw pointer
         */
        [[nodiscard]] T** operator&()
        {
            if (ptr) throw new std::runtime_error("Acquire address for non-null pointer");
            return &ptr;
        }

        explicit operator bool() const noexcept
        {
            return ptr != nullptr;
        }
    };

    template <class T>
    class ComQIPtr : public ComPtr<T> {
      protected:
        const IID iid;

      public:
        constexpr ComQIPtr(const IID iid) noexcept : iid(iid) {}
        constexpr ComQIPtr(const IID iid, std::nullptr_t) noexcept : iid(iid) {}

        template <class Q>
        ComQIPtr(const IID iid, const ComPtr<Q>& other) noexcept : iid(iid)
        {
            IUnknown* p;
            if (!other || FAILED(other->QueryInterface(iid, (void**)&p))) { this->ptr = nullptr; }
            else {
                this->ptr = reinterpret_cast<T*>(p);
            }
        }

        template <class Q>
        ComQIPtr& operator=(const ComPtr<Q>& other) noexcept
        {
            ComQIPtr(iid, other).swap(*this);
            return *this;
        }
    };
}// namespace libtf