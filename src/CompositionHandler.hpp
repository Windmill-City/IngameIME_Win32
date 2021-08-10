#pragma once
#include "CommitHandler.hpp"
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

extern "C"
{
    typedef enum CompositionState
    {
        CompositionBegin,
        CompositionUpdate,
        CompositionEnd
    } CompositionState_t;
    typedef RECT BoundingBox_t;
    typedef BSTR PreEdit;
    typedef void (*CallbackComposition)(HWND, CompositionState_t, PreEdit);
    typedef HRESULT (*CallbackBoundingBox)(HWND, BoundingBox_t *);
}

namespace libtf
{
    class CompositionHandler : public CComObjectRoot, public ITfContextOwnerCompositionSink
    {
    protected:
        TfClientId m_clientId;
        CComPtr<ITfContext> m_context;
        TfEditCookie m_ecTextStore;

        /**
         * @brief Get the current window of the context
         * 
         * @param hWnd receive window handle
         * @return HRESULT 
         */
        HRESULT getWnd(HWND &hWnd)
        {
            CComQIPtr<ITfContextOwner> contextOwner = m_context;
            return contextOwner->GetWnd(&hWnd);
        }

    public:
        /**
         * @brief Callback when PreEdit updates
         */
        typedef std::function<void(HWND, CompositionState_t, PreEdit)> signalComposition;
        signalComposition m_sigComposition = [](HWND, CompositionState_t, PreEdit) {};

        /**
         * @brief Callback to get PreEdit's BoundingBox
         * for positioning Candidate List window
         */
        typedef std::function<void(HWND, BoundingBox_t *)> signalBoundingBox;
        signalBoundingBox m_sigBoundingBox = [](HWND, BoundingBox_t *) {};

        /**
         * @brief Handle input method commit str
         */
        CComPtr<CCommitHandler> m_commitHandler = new CCommitHandler();

        BEGIN_COM_MAP(CompositionHandler)
        COM_INTERFACE_ENTRY(ITfContextOwnerCompositionSink)
        END_COM_MAP()

        /**
         * @brief Initialize handler
         * 
         * @param clientId ClientId to request edit session
         * @param context ITfContext to request edit session
         * @param ec Read Only EditCookie to get PreEdit str
         * @return HRESULT 
         */
        HRESULT initialize(TfClientId clientId, CComPtr<ITfContext> context, TfEditCookie ec)
        {
            m_clientId = clientId;
            m_context = context;
            m_ecTextStore = ec;
            CHECK_HR(m_commitHandler->initialize(m_context));
            return S_OK;
        }

        /**
         * @brief Dispose the handler
         * 
         * @return HRESULT 
         */
        HRESULT dispose()
        {
            CHECK_HR(m_commitHandler->dispose());
            return S_OK;
        }

#pragma region ITfContextOwnerCompositionSink
        /**
         * @brief Always allow Composition start
         * Call callback with empty PreEdit
         * 
         * @param pfOk allow Composition start?
         * @return HRESULT 
         */
        HRESULT OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) override
        {
            //Always allow Composition start
            *pfOk = TRUE;

            //Empty PreEdit
            BSTR bstr = SysAllocString(L"");

            HWND hWnd;
            getWnd(hWnd);

            m_sigComposition(hWnd, CompositionState::CompositionBegin, bstr);

            //Cleanup
            SysReleaseString(bstr);

            return S_OK;
        }

        /**
         * @brief Get PreEdit str and call callback with it
         * 
         * @return HRESULT 
         */
        HRESULT OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew) override
        {
            CComPtr<ITfRange> range;
            CHECK_HR(pComposition->GetRange(&range));

            ULONG charCount;
            WCHAR *buf = new WCHAR[65];
            CHECK_OOM(buf);
            ZeroMemory(buf, sizeof(buf));

            CHECK_HR(range->GetText(m_ecTextStore, 0, buf, 64, &charCount));

            BSTR bstr = SysAllocString(buf);

            HWND hWnd;
            getWnd(hWnd);

            m_sigComposition(hWnd, CompositionState::CompositionUpdate, bstr);

            //Cleanup
            SysReleaseString(bstr);
            delete[] buf;

            return S_OK;
        }

        /**
         * @brief Call callback with empty PreEdit
         * 
         * @return HRESULT 
         */
        HRESULT OnEndComposition(ITfCompositionView *pComposition) override
        {
            //Empty PreEdit
            BSTR bstr = SysAllocString(L"");

            HWND hWnd;
            getWnd(hWnd);

            m_sigComposition(hWnd, CompositionState::CompositionEnd, bstr);

            //Cleanup
            SysReleaseString(bstr);

            HRESULT hr;
            CHECK_HR(m_context->RequestEditSession(m_clientId, m_commitHandler, TF_ES_ASYNC | TF_ES_READWRITE, &hr));

            return hr;
        }
#pragma endregion
    };

    typedef CComObjectNoLock<CompositionHandler> CCompositionHandler;
}