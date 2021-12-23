#pragma once
#include <atlbase.h>
#include <atlcom.h>

#include <msctf.h>

#include "CandidateListHandler.hpp"

namespace libtf {
    class FullScreenUIElementHandler : public CComObjectRoot, public ITfUIElementSink {
        CComQIPtr<ITfUIElementMgr> m_UIElementMgr;
        DWORD                      m_UIElementMgrSinkCookie         = TF_INVALID_COOKIE;
        DWORD                      m_ActiveCandidateListUIElementId = 0;
        bool                       m_FullScreenMode                 = false;

      public:
        const std::unique_ptr<CandidateListHandler> m_CandidateListHandler = std::make_unique<CandidateListHandler>();

      public:
        /**
         * @brief Create the handler
         *
         * @param uiElementMgr Query interface from ITfThreadMgr
         * @return HRESULT
         */
        HRESULT ctor(CComPtr<ITfThreadMgr> threadMgr)
        {
            m_UIElementMgr              = threadMgr;
            CComQIPtr<ITfSource> source = m_UIElementMgr;
            return source->AdviseSink(IID_ITfUIElementSink, this, &m_UIElementMgrSinkCookie);
        }

        /**
         * @brief Free event sink
         *
         * @return HRESULT
         */
        HRESULT dtor()
        {
            BEGIN_HRESULT();
            if (m_UIElementMgrSinkCookie == TF_INVALID_COOKIE) return S_OK;
            CComQIPtr<ITfSource> source = m_UIElementMgr;
            CHECK_HR(source->UnadviseSink(m_UIElementMgrSinkCookie));
            m_UIElementMgrSinkCookie = TF_INVALID_COOKIE;
            END_HRESULT();
        }

      public:
        BEGIN_COM_MAP(FullScreenUIElementHandler)
        COM_INTERFACE_ENTRY(ITfUIElementSink)
        END_COM_MAP()

        /**
         * @brief Set if the game is in fullScreen mode
         *
         * @param fullScreen is game window in fullScreen mode?
         * @return HRESULT
         */
        HRESULT setFullScreen(const bool fullScreen)
        {
            m_FullScreenMode = fullScreen;
            return S_OK;
        }

        /**
         * @brief Get context fullScreen state
         *
         * @param fullScreen receive fullScreen state
         * @return E_INVALIDARG if fullScreen is NULL
         */
        HRESULT getFullScreen(bool* fullScreen) const
        {
            if (!fullScreen) return E_INVALIDARG;
            *fullScreen = m_FullScreenMode;
            return S_OK;
        }

        /**
         * @brief Hide all the windows of input method in full screen mode
         */
        HRESULT STDMETHODCALLTYPE BeginUIElement(DWORD dwUIElementId, BOOL* pbShow) override
        {
            BEGIN_HRESULT();

            *pbShow = !m_FullScreenMode;

            CComPtr<ITfUIElement> uiElement;
            CHECK_HR(m_UIElementMgr->GetUIElement(dwUIElementId, &uiElement));

            if (uiElement) {
                CComQIPtr<ITfCandidateListUIElement> candidateListUIElement;
                // Check if current UIElement is CandidateListUIElement
                if (candidateListUIElement = uiElement) {
                    m_ActiveCandidateListUIElementId = dwUIElementId;
                    // Handle Candidate List events
                    CHECK_HR(m_CandidateListHandler->BeginUIElement(candidateListUIElement));
                }
            }
            END_HRESULT();
        }

        HRESULT STDMETHODCALLTYPE UpdateUIElement(DWORD dwUIElementId) override
        {
            if (dwUIElementId != m_ActiveCandidateListUIElementId) return S_OK;
            return m_CandidateListHandler->UpdateUIElement();
        }

        HRESULT STDMETHODCALLTYPE EndUIElement(DWORD dwUIElementId) override
        {
            if (dwUIElementId != m_ActiveCandidateListUIElementId) return S_OK;
            return m_CandidateListHandler->EndUIElement();
        }
    };

    typedef CComObjectNoLock<FullScreenUIElementHandler> CFullScreenUIElementHandler;
}// namespace libtf