#pragma once

#include <msctf.h>
#include "tf_common.h"

#define TF_COM_AS(Interface) \
if (IsEqualIID(IID_##Interface, riid)) {	\
	*ppvObject = (Interface*)this;	\
}

#define TF_COM_ASUNK(Interface) \
if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_##Interface, riid)) {	\
	*ppvObject = (Interface*)this;	\
}

#define TF_COM_RETURN \
if (*ppvObject) {	\
	AddRef();	\
	return S_OK;	\
}	\
return E_NOINTERFACE;

#define TF_COM_REFS \
virtual ULONG __stdcall AddRef(void) override { return COMBase::AddRef(); };	\
virtual ULONG __stdcall Release(void) override { return COMBase::Release(); };

class TFAPI COMBase :
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
