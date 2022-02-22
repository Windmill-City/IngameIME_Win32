#pragma once

#include <functional>
#include <memory>

#include <msctf.h>

namespace libtf {
    typedef HRESULT(WINAPI* pTF_CreateThreadMgr)(ITfThreadMgr**);
    typedef HRESULT(WINAPI* pTF_CreateInputProcessorProfiles)(ITfInputProcessorProfiles**);
    typedef HRESULT(WINAPI* pTF_GetThreadMgr)(ITfThreadMgr**);

    auto getMsCtf()
    {
        return std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)>(LoadLibraryW(L"msctf.dll"),
                                                                                         ::FreeLibrary);
    }

    /**
     * @brief Create ThreadMgr for the calling thread without initializing COM
     *
     * ThreadMgr created by COM is using Apartment Model, which makes it problematic in MultiThreaded Model
     */
    HRESULT createThreadMgr(ITfThreadMgr** pptim)
    {
        auto hMsCtf = getMsCtf();
        if (!hMsCtf) return E_FAIL;
        auto pfn = (pTF_CreateThreadMgr)GetProcAddress(hMsCtf.get(), "TF_CreateThreadMgr");

        return pfn ? (*pfn)(pptim) : E_FAIL;
    }

    /**
     * @brief Obtains a copy of ThreadMgr previously created within the calling thread,
     * or create new if no previous one
     */
    HRESULT getThreadMgr(ITfThreadMgr** pptim)
    {
        auto hMsCtf = getMsCtf();
        if (!hMsCtf) return E_FAIL;
        auto pfn = (pTF_GetThreadMgr)GetProcAddress(hMsCtf.get(), "TF_GetThreadMgr");
        if (!pfn) return E_FAIL;

        HRESULT hr;

        if (SUCCEEDED(hr = (*pfn)(pptim))) return hr;

        return createThreadMgr(pptim);
    }

    /**
     * @brief Create InputProcessorProfiles for the calling thread without initializing COM
     *
     * InputProcessorProfiles created by COM is using Apartment Model, which makes it problematic in MultiThreaded Model
     */
    HRESULT createInputProcessorProfiles(ITfInputProcessorProfiles** ppipr)
    {
        auto hMsCtf = getMsCtf();
        if (!hMsCtf) return E_FAIL;
        auto pfn = (pTF_CreateInputProcessorProfiles)GetProcAddress(hMsCtf.get(), "TF_CreateInputProcessorProfiles");

        return pfn ? (*pfn)(ppipr) : E_FAIL;
    }
}// namespace libtf