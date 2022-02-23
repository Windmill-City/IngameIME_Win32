#pragma once
#include "ComException.hpp"
#include <Unknwn.h>

#define COM_DEF_BEGIN()                                                                                                \
    virtual HRESULT STDMETHODCALLTYPE QueryInterface(                                                                  \
        /* [in] */ REFIID riid, /* [iid_is][out] */ _COM_Outptr_ void __RPC_FAR* __RPC_FAR* ppvObject) override        \
    {                                                                                                                  \
        if (!ppvObject) return E_POINTER;                                                                              \
        *ppvObject = nullptr;                                                                                          \
        if (IsEqualIID(IID_IUnknown, riid)) *ppvObject = this;
#define COM_DEF_INF(InfName)                                                                                           \
    if (IsEqualIID(IID_##InfName, riid)) *ppvObject = static_cast<InfName*>(this);
#define COM_DEF_END()                                                                                                  \
    if (*ppvObject) {                                                                                                  \
        this->AddRef();                                                                                                \
        return S_OK;                                                                                                   \
    }                                                                                                                  \
    return E_NOINTERFACE;                                                                                              \
    }                                                                                                                  \
    virtual ULONG STDMETHODCALLTYPE AddRef(void) override                                                              \
    {                                                                                                                  \
        return ComObjectBase::InternalAddRef();                                                                        \
    }                                                                                                                  \
    virtual ULONG STDMETHODCALLTYPE Release(void) override                                                             \
    {                                                                                                                  \
        return ComObjectBase::InternalRelease();                                                                       \
    }

#define COM_HR_BEGIN(DefaultHR)                                                                                        \
    HRESULT hr = DefaultHR;                                                                                            \
    do {
#define COM_HR_END()                                                                                                   \
    }                                                                                                                  \
    while (0)                                                                                                          \
        ;
#define THR_HR(hr) throw libtf::ComException(hr);
#define CHECK_HR(exp)                                                                                                  \
    if (FAILED(hr = (exp))) break;
#define COM_HR_THR()                                                                                                   \
    if (FAILED(hr)) THR_HR(hr);
#define COM_HR_RET() return hr;

namespace libtf {
    class ComObjectBase {
      private:
        /**
         * @brief Reference count of the ComObj
         *
         */
        DWORD ref = 0;

      public:
        virtual ~ComObjectBase() = default;

      protected:
        ULONG InternalAddRef(void)
        {
            return ++ref;
        }

        ULONG InternalRelease(void)
        {
            if (--ref == 0) delete this;
            if (ref < 0) throw new std::exception("Double-free of COM Object");
            return ref;
        }
    };
}// namespace libtf