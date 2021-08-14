#pragma once

#include <winerror.h>

#define LIBTF_EXPORT __declspec(dllexport)

#define CHECK_HR(hresultItem)                                                                                          \
    {                                                                                                                  \
        HRESULT _hr = hresultItem;                                                                                     \
        if (FAILED(_hr)) return _hr;                                                                                   \
    }

/**
 * @brief Check if ERROR_SUCCESS
 */
#define NOT_ES(hresultItem) (ERROR_SUCCESS != (hresultItem))

/**
 * @brief Check if ERROR_SUCCESS
 */
#define CHECK_ES(hresultItem)                                                                                          \
    {                                                                                                                  \
        HRESULT _hr = hresultItem;                                                                                     \
        if (NOT_ES(_hr)) return _hr;                                                                                   \
    }

#define CHECK_OOM(ptr)                                                                                                 \
    if (!(ptr)) return E_OUTOFMEMORY;