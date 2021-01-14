#pragma once
#include "Common.hpp"
namespace libtf {
#define COM_AS(Interface) \
if (IsEqualIID(IID_##Interface, riid)) {	\
	*ppvObject = (Interface*)this;	\
}

#define COM_ASUNK(Interface) \
if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_##Interface, riid)) {	\
	*ppvObject = (Interface*)this;	\
}

#define COM_RETURN \
if (*ppvObject) {	\
	COMBase::AddRef();	\
	return S_OK;	\
}	\
return E_NOINTERFACE;

#define COM_REFS \
ULONG __stdcall AddRef(void) override { return COMBase::AddRef(); };	\
ULONG __stdcall Release(void) override { return COMBase::Release(); };

	class COMBase : public IUnknown
	{
	public:
		ULONG __stdcall IUnknown::AddRef(void) override {
			return ++m_ulRef;
		};
		ULONG __stdcall IUnknown::Release(void) override {
			if (--m_ulRef <= 0) {
				delete this;
				return 0;
			}
			return m_ulRef;
		};
	private:
		ULONG m_ulRef = 0;
	};
}
