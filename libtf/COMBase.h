#pragma once
#include <msctf.h>

#define TF_COM_AS(Interface) \
if (IsEqualIID(IID_##Interface, riid)) {	\
	*ppvObject = (Interface*)this;	\
}

#define TF_COM_ASUNK(Interface) \
if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_##Interface, riid)) {	\
	*ppvObject = (Interface*)this;	\
}

#define TF_COM_QUERYINF(CLASSNAME , ...) \
HRESULT __stdcall CLASSNAME::QueryInterface(REFIID riid, void** ppvObject)	\
{	\
	if (ppvObject == NULL) return E_INVALIDARG;	\
	*ppvObject = NULL;	\
	__VA_ARGS__		\
	if (*ppvObject)	\
	{	\
	COMBase::AddRef();	\
	return S_OK;	\
	}	\
	return E_NOINTERFACE;	\
}

#define TF_COM_REFS \
virtual ULONG __stdcall AddRef(void) override { return COMBase::AddRef(); };	\
virtual ULONG __stdcall Release(void) override { return COMBase::Release(); };

class COMBase :
	public IUnknown
{
public:
	//IUnknown
	virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
		return E_NOTIMPL;
	};
	virtual ULONG __stdcall AddRef(void) override {
		return ++m_ulRef;
	};
	virtual ULONG __stdcall Release(void) override {
		if (--m_ulRef <= 0) {
			delete this;
			return 0;
		}
		return m_ulRef;
	};
private:
	ULONG m_ulRef;
};
