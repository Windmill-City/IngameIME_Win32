#pragma once
#include "libtfdef.h"
#include <atlbase.h>
#include <atlcom.h>
#include <functional>
#include <msctf.h>

extern "C"
{
    typedef BSTR libtf_Commit;
    typedef void (*libtf_CallbackCommit)(libtf_Commit);
}

namespace libtf
{
    class CommitHandler : public CComObjectRoot, public ITfEditSession
    {
    protected:
        CComPtr<ITfContext> m_context;

    public:
        /**
         * @brief Callback when input method Commit
         */
        typedef std::function<void(libtf_Commit)> signalCommit;
        signalCommit m_sigCommit = [](libtf_Commit) {};

        BEGIN_COM_MAP(CommitHandler)
        COM_INTERFACE_ENTRY(ITfEditSession)
        END_COM_MAP()

        /**
         * @brief Initialize handler
         * 
         * @param context ITfContext to get Commit
         * @return HRESULT 
         */
        HRESULT initialize(CComPtr<ITfContext> context)
        {
            m_context = context;
            return S_OK;
        }

        /**
         * @brief Dispose the handler
         * 
         * @return HRESULT 
         */
        HRESULT dispose()
        {
            return S_OK;
        }

        /**
         * @brief Get all the text in the context, which is Commit
         */
        HRESULT DoEditSession(TfEditCookie ec) override
        {
            //Get a range which cover all the text in the context
            CComPtr<ITfRange> fullRange;
            CComPtr<ITfRange> rangeAtEnd;
            CHECK_HR(m_context->GetStart(ec, &fullRange));
            CHECK_HR(m_context->GetEnd(ec, &rangeAtEnd));
            CHECK_HR(fullRange->ShiftEndToRange(ec, rangeAtEnd, TF_ANCHOR_END));

            BOOL isEmpty;
            CHECK_HR(fullRange->IsEmpty(ec, &isEmpty));
            if (isEmpty) return S_OK;

            ULONG charCount;
            WCHAR *buf = new WCHAR[65];
            CHECK_OOM(buf);
            ZeroMemory(buf, sizeof(buf));

            CHECK_HR(fullRange->GetText(ec, 0, buf, 64, &charCount));
            BSTR bstr = SysAllocString(buf);

            m_sigCommit(bstr);

            //Cleanup
            SysFreeString(bstr);
            delete[] buf;

            //Clean the handled Commit str
            CHECK_HR(fullRange->SetText(ec, 0, NULL, 0));

            return S_OK;
        }
    };

    typedef CComObjectNoLock<CommitHandler> CCommitHandler;
}