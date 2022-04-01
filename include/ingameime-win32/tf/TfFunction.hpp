#pragma once
#include <functional>
#include <memory>

#include <msctf.h>
#include <windows.h>

namespace IngameIME::tf
{
using pTF_CreateThreadMgr              = HRESULT(WINAPI*)(ITfThreadMgr**);
using pTF_CreateInputProcessorProfiles = HRESULT(WINAPI*)(ITfInputProcessorProfiles**);
using pTF_GetThreadMgr                 = HRESULT(WINAPI*)(ITfThreadMgr**);

/**
 * @brief Create ThreadMgr for the calling thread without initializing COM
 *
 * ThreadMgr created by COM is using Apartment Model, which makes it problematic in MultiThreaded Model
 */
HRESULT createThreadMgr(ITfThreadMgr** pptim);
/**
 * @brief Obtains a copy of ThreadMgr previously created within the calling thread,
 * or create new if no previous one
 */
HRESULT getThreadMgr(ITfThreadMgr** pptim);
/**
 * @brief Create InputProcessorProfiles for the calling thread without initializing COM
 *
 * InputProcessorProfiles created by COM is using Apartment Model, which makes it problematic in MultiThreaded Model
 */
HRESULT createInputProcessorProfiles(ITfInputProcessorProfiles** ppipr);
} // namespace IngameIME::tf