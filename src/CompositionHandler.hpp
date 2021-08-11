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
    typedef struct Composition
    {
        CompositionState_t state;
        /**
         * @brief Only Available at CompositionUpdate
         */
        PreEdit preEdit;
        /**
         * @brief Only Available at CompositionUpdate
         */
        long selection[2];
    } Composition_t, *pComposition;
    typedef void (*CallbackComposition)(Composition_t);
    typedef HRESULT (*CallbackBoundingBox)(BoundingBox_t *);
}

namespace libtf
{
    class CompositionHandler : public CComObjectRoot, public ITfContextOwnerCompositionSink
    {
    protected:
        TfClientId m_clientId;
        CComPtr<ITfContext> m_context;
        TfEditCookie m_ecTextStore;

    public:
        /**
         * @brief Callback when PreEdit updates
         */
        typedef std::function<void(Composition_t)> signalComposition;
        signalComposition m_sigComposition = [](Composition_t) {};

        /**
         * @brief Callback to get PreEdit's BoundingBox
         * for positioning Candidate List window
         */
        typedef std::function<void(BoundingBox_t *)> signalBoundingBox;
        signalBoundingBox m_sigBoundingBox = [](BoundingBox_t *) {};

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
         * 
         * @param pfOk allow Composition start?
         * @return HRESULT 
         */
        HRESULT OnStartComposition(ITfCompositionView *pComposition, BOOL *pfOk) override
        {
            //Always allow Composition start
            *pfOk = TRUE;

            m_sigComposition({CompositionBegin});

            return S_OK;
        }

        /**
         * @brief Get Composition data and call callback with it
         * 
         * @return HRESULT 
         */
        HRESULT OnUpdateComposition(ITfCompositionView *pComposition, ITfRange *pRangeNew) override
        {
            CComPtr<ITfRange> preEditRange;
            CHECK_HR(pComposition->GetRange(&preEditRange));

            ULONG charCount;
            WCHAR *buf = new WCHAR[65];
            CHECK_OOM(buf);
            ZeroMemory(buf, sizeof(buf));

            CHECK_HR(preEditRange->GetText(m_ecTextStore, 0, buf, 64, &charCount));
            BSTR bstr = SysAllocString(buf);

            CComQIPtr<ITfRangeACP> preEditRangeAcp = preEditRange;
            LONG acpStart, len;
            CHECK_HR(preEditRangeAcp->GetExtent(&acpStart, &len));

            Composition_t composition = {CompositionUpdate, bstr, {acpStart, acpStart + len}};
            m_sigComposition(composition);

            //Cleanup
            SysReleaseString(bstr);
            delete[] buf;

            return S_OK;
        }

        /**
         * @brief Handle Composition End
         * 
         * @return HRESULT 
         */
        HRESULT OnEndComposition(ITfCompositionView *pComposition) override
        {
            m_sigComposition({CompositionEnd});

            HRESULT hr;
            CHECK_HR(m_context->RequestEditSession(m_clientId, m_commitHandler, TF_ES_ASYNC | TF_ES_READWRITE, &hr));
            return hr;
        }
#pragma endregion
    };

    typedef CComObjectNoLock<CompositionHandler> CCompositionHandler;
}