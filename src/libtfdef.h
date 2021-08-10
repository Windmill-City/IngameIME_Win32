#pragma once

#include <winerror.h>

#define CHECK_HR(hresultItem)        \
    {                                \
        HRESULT _hr = hresultItem;   \
        if (FAILED(_hr)) return _hr; \
    }

#define CHECK_OOM(ptr) \
    if (!(ptr)) return E_OUTOFMEMORY;