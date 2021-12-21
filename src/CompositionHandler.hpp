#pragma once
#include <string>

#include <atlbase.h>
#include <atlcom.h>

#include <msctf.h>

#include "libtfdef.h"

#include "ICallback.hpp"
#include "IThreadAssociate.hpp"

namespace libtf {
    class CommitHandler : public CComObjectRoot, public ITfEditSession, public ICallback<std::wstring> {
        CComPtr<ITfContext> m_Context;

      public:
        /**
         * @brief Create the handler
         *
         * @param context ITfContext to fetch Commit
         * @return HRESULT
         */
        HRESULT ctor(CComPtr<ITfContext> context)
        {
            m_Context = context;
            return S_OK;
        }

      public:
        BEGIN_COM_MAP(CommitHandler)
        COM_INTERFACE_ENTRY(ITfEditSession)
        END_COM_MAP()

        /**
         * @brief Get all the text in the context, which is commit string
         */
        HRESULT STDMETHODCALLTYPE DoEditSession(TfEditCookie ec) override
        {
            BEGIN_HRESULT();
            // Get a range which covers all the texts in the context
            CComPtr<ITfRange> fullRange;
            CComPtr<ITfRange> rangeAtEnd;
            CHECK_HR(m_Context->GetStart(ec, &fullRange));
            CHECK_HR(m_Context->GetEnd(ec, &rangeAtEnd));
            CHECK_HR(fullRange->ShiftEndToRange(ec, rangeAtEnd, TF_ANCHOR_END));

            // It's possible that the context is empty when there is no commit
            BOOL isEmpty;
            CHECK_HR(fullRange->IsEmpty(ec, &isEmpty));
            if (isEmpty) return S_OK;

            ULONG commitLength;
            // Get the text length
            CHECK_HR(fullRange->GetText(ec, 0, NULL, 0, &commitLength));
            auto bufCommit = std::make_unique<WCHAR[]>(commitLength);
            // Get the commit text
            CHECK_HR(fullRange->GetText(ec, 0, bufCommit.get(), commitLength, &commitLength));
            // Clear the texts in the text store
            CHECK_HR(fullRange->SetText(ec, 0, NULL, 0));

            runCallback(std::wstring(bufCommit.get(), commitLength));

            END_HRESULT();
        }
    };
    typedef CComObjectNoLock<CommitHandler> CCommitHandler;

    class PreEditContext {
      public:
        /**
         * @brief Start index of the selection(inclusive)
         */
        uint32_t m_SelStart;
        /**
         * @brief End index of the selection(exclusive)
         */
        uint32_t m_SelEnd;
        /**
         * @brief Zero-terminated string
         */
        std::wstring m_Content;

      public:
        PreEditContext(uint32_t selStart, uint32_t selEnd, std::wstring content)
        {
            m_SelStart = selStart;
            m_SelEnd   = selEnd;
            m_Content  = std::move(content);
        }
    };
    typedef ICallback<const libtf_CompositionState_t, std::unique_ptr<const PreEditContext>> PreEditContextCallback;
    /**
     * @brief Return the boundary rectangle of the preedit, in window coordinate
     *
     * @note If the length of preedit is 0 (as it would be drawn by input method), the rectangle
     * coincides with the insertion point, and its width is 0.
     */
    typedef ICallback<RECT*> PreEditRectCallback;
    class PreEditHandler : public CComObjectRoot,
                           public ITfTextEditSink,
                           public PreEditContextCallback,
                           public PreEditRectCallback {
        CComPtr<ITfContext>         m_Context;
        DWORD                       m_TextEditSinkCookie = TF_INVALID_COOKIE;
        CComPtr<ITfCompositionView> m_CompositionView;
        friend class CompositionHandler;

      public:
        /**
         * @brief Create the handler
         *
         * @param context ITfContext to fetch Commit
         * @return HRESULT
         */
        HRESULT ctor(CComPtr<ITfContext> context)
        {
            m_Context = context;

            // Listen for preedit change
            CComQIPtr<ITfSource> source = context;
            return source->AdviseSink(IID_ITfTextEditSink, this, &m_TextEditSinkCookie);
        }

        /**
         * @brief Free event sink
         *
         * @return HRESULT
         */
        HRESULT dtor()
        {
            BEGIN_HRESULT();

            if (m_TextEditSinkCookie == TF_INVALID_COOKIE) return S_OK;

            CComQIPtr<ITfSource> source = m_Context;
            CHECK_HR(source->UnadviseSink(m_TextEditSinkCookie));
            m_TextEditSinkCookie = TF_INVALID_COOKIE;

            END_HRESULT();
        }

      public:
        BEGIN_COM_MAP(PreEditHandler)
        COM_INTERFACE_ENTRY(ITfTextEditSink)
        END_COM_MAP()

        /**
         * @brief Get PreEdit text and its selection
         *
         * @note Selection change only triggers OnEndEdit event,
         *  for convenient, we handle preedit here at the same time
         */
        HRESULT STDMETHODCALLTYPE OnEndEdit(ITfContext* pic, TfEditCookie ec, ITfEditRecord* pEditRecord) override
        {
            BEGIN_HRESULT();

            // No active composition
            if (!m_CompositionView) return S_OK;

            CComPtr<ITfRange> preEditRange;
            CHECK_HR(m_CompositionView->GetRange(&preEditRange));

            // Get preedit length
            CComQIPtr<ITfRangeACP> rangeAcp = preEditRange;
            LONG                   acpStart, len;
            CHECK_HR(rangeAcp->GetExtent(&acpStart, &len));
            ULONG preEditLength = len;
            auto  bufPreEdit    = std::make_unique<WCHAR[]>(preEditLength);
            // Get preedit text
            CHECK_HR(preEditRange->GetText(ec, 0, bufPreEdit.get(), preEditLength, &preEditLength));

            // Get selection of the preedit
            TF_SELECTION      sel[1];
            ULONG             fetched;
            CComPtr<ITfRange> selRange;
            CHECK_HR(m_Context->GetSelection(ec, TF_DEFAULT_SELECTION, 1, sel, &fetched));
            selRange.Attach(sel[0].range);
            rangeAcp = selRange;
            CHECK_HR(rangeAcp->GetExtent(&acpStart, &len));

            PreEditContextCallback::runCallback(
                libtf_CompositionUpdate,
                std::make_unique<PreEditContext>(
                    acpStart, acpStart + len, std::wstring(bufPreEdit.get(), preEditLength)));

            END_HRESULT();
        }
    };
    typedef CComObjectNoLock<PreEditHandler> CPreEditHandler;

    class CompositionHandler : public CComObjectRoot, public ITfContextOwnerCompositionSink, public IThreadAssociate {
        TfClientId                                    m_ClientId;
        CComPtr<ITfContext>                           m_Context;
        CComQIPtr<ITfContextOwnerCompositionServices> m_Services;

      public:
        const CComPtr<CPreEditHandler> m_PreEditHandler = new CPreEditHandler();
        const CComPtr<CCommitHandler>  m_CommitHandler  = new CCommitHandler();

      public:
        /**
         * @brief Create the handler
         *
         * @param context ITfContext to fetch Commit
         * @return HRESULT
         */
        HRESULT ctor(TfClientId clientId, CComPtr<ITfContext> context)
        {
            BEGIN_HRESULT();

            initialCreatorThread();

            m_ClientId = clientId;
            m_Context  = context;
            m_Services = context;

            CHECK_HR(m_PreEditHandler->ctor(context));
            CHECK_HR(m_CommitHandler->ctor(context));

            END_HRESULT();
        }

        /**
         * @brief Free event sink
         *
         * @return HRESULT
         */
        HRESULT dtor()
        {
            BEGIN_HRESULT();

            CHECK_HR(terminate());

            CHECK_HR(m_PreEditHandler->dtor());

            END_HRESULT();
        }

      public:
        BEGIN_COM_MAP(CompositionHandler)
        COM_INTERFACE_ENTRY(ITfContextOwnerCompositionSink)
        END_COM_MAP()

        /**
         * @brief Terminate active composition
         *
         * @return UI_E_WRONG_THREAD if the calling thread isn't the thread that create the context
         */
        HRESULT terminate() const
        {
            BEGIN_HRESULT();
            CHECK_HR(assertCreatorThread());
            // Pass NULL to terminate all the composition
            CHECK_HR(m_Services->TerminateComposition(NULL));
            END_HRESULT();
        }

#pragma region ITfContextOwnerCompositionSink
        HRESULT STDMETHODCALLTYPE OnStartComposition(ITfCompositionView* pComposition, BOOL* pfOk) override
        {
            // Always allow Composition start
            *pfOk = TRUE;
            m_PreEditHandler->PreEditContextCallback::runCallback(libtf_CompositionBegin, NULL);
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnUpdateComposition(ITfCompositionView* pComposition, ITfRange* pRangeNew) override
        {
            // Handle preedit in ITfTextEditSink
            m_PreEditHandler->m_CompositionView = pComposition;
            return S_OK;
        }

        HRESULT STDMETHODCALLTYPE OnEndComposition(ITfCompositionView* pComposition) override
        {
            m_PreEditHandler->m_CompositionView.Release();
            m_PreEditHandler->PreEditContextCallback::runCallback(libtf_CompositionEnd, NULL);

            BEGIN_HRESULT();
            CHECK_HR(m_Context->RequestEditSession(m_ClientId, m_CommitHandler, TF_ES_ASYNC | TF_ES_READWRITE, NULL));
            END_HRESULT();
        }
#pragma endregion
    };

    typedef CComObjectNoLock<CompositionHandler> CCompositionHandler;
}// namespace libtf