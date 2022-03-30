#include "tf/TfFunction.hpp"

namespace IngameIME::tf

{

auto getMsCtf()
{
    return std::unique_ptr<std::remove_pointer_t<HMODULE>, decltype(&::FreeLibrary)>(LoadLibraryW(L"msctf.dll"),
                                                                                     ::FreeLibrary);
}

HRESULT createThreadMgr(ITfThreadMgr** pptim)
{
    auto hMsCtf = getMsCtf();
    if (!hMsCtf) return E_FAIL;
    auto pfn = (pTF_CreateThreadMgr)GetProcAddress(hMsCtf.get(), "TF_CreateThreadMgr");

    return pfn ? (*pfn)(pptim) : E_FAIL;
}

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

HRESULT createInputProcessorProfiles(ITfInputProcessorProfiles** ppipr)
{
    auto hMsCtf = getMsCtf();
    if (!hMsCtf) return E_FAIL;
    auto pfn = (pTF_CreateInputProcessorProfiles)GetProcAddress(hMsCtf.get(), "TF_CreateInputProcessorProfiles");

    return pfn ? (*pfn)(ppipr) : E_FAIL;
}

} // namespace IngameIME::tf
