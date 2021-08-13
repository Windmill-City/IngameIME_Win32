#pragma once
#include "libtfdef.h"

#include <msctf.h>

namespace libtf {
    typedef HRESULT(WINAPI* pTF_CreateThreadMgr)(ITfThreadMgr**);
    typedef HRESULT(WINAPI* pTF_CreateInputProcessorProfiles)(ITfInputProcessorProfiles**);
    typedef HRESULT(WINAPI* pTF_GetThreadMgr)(ITfThreadMgr**);

    /**
     * @brief Do operation with msctf module
     *
     * @param toDo A function that receives HMODULE objects and returns HRESULT
     */
    template <typename F>
    HRESULT withMsCtf(F toDo)
    {
        HMODULE hMsCtf = LoadLibrary(TEXT("msctf.dll"));
        // Error loading module
        if (!hMsCtf) return E_FAIL;

        HRESULT hr = toDo(hMsCtf);

        // Cleanup
        FreeLibrary(hMsCtf);

        return hr;
    }

    /**
     * @brief Create a ThreadMgr object for the calling thread without initializing COM
     *
     * Why not using CoCreateInstance?
     * The instance created by the COM using Apartment Thread Model, which can't be use in a
     * MultiThreaded environment
     */
    HRESULT createThreadMgr(ITfThreadMgr** pptim)
    {
        return withMsCtf([pptim](HMODULE hMsCtf) {
            pTF_CreateThreadMgr pfnCreateThreadMgr = (pTF_CreateThreadMgr)GetProcAddress(hMsCtf, "TF_CreateThreadMgr");
            if (!pfnCreateThreadMgr) return E_FAIL;

            CHECK_HR((*pfnCreateThreadMgr)(pptim));

            return S_OK;
        });
    }

    /**
     * @brief Create a InputProcessorProfiles object for the calling thread without initializing COM
     *
     * @see createThreadMgr for the reason why we dont use COM
     */
    HRESULT createInputProcessorProfiles(ITfInputProcessorProfiles** ppipr)
    {
        return withMsCtf([ppipr](HMODULE hMsCtf) {
            pTF_CreateInputProcessorProfiles pfnCreateInputProcessorProfiles =
                (pTF_CreateInputProcessorProfiles)GetProcAddress(hMsCtf, "TF_CreateInputProcessorProfiles");
            if (!pfnCreateInputProcessorProfiles) return E_FAIL;

            CHECK_HR((*pfnCreateInputProcessorProfiles)(ppipr));

            return S_OK;
        });
    }

    /**
     * @brief Obtains a copy of a thread manager object previously created
     * within the calling thread
     */
    HRESULT getThreadMgr(ITfThreadMgr** pptim)
    {
        return withMsCtf([pptim](HMODULE hMsCtf) {
            pTF_GetThreadMgr pfnGetThreadMgr = (pTF_GetThreadMgr)GetProcAddress(hMsCtf, "TF_GetThreadMgr");
            if (!pfnGetThreadMgr) return E_FAIL;

            CHECK_HR((*pfnGetThreadMgr)(pptim));

            return S_OK;
        });
    }
}// namespace libtf