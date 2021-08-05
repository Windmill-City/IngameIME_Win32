#pragma once
#include <stdint.h>
#include <unknwn.h>
namespace libtf
{
#define COM_QUERY_INF_BEGIN(Interface)                                       \
	if (IsEqualIID(IID_IUnknown, riid) || IsEqualIID(IID_##Interface, riid)) \
	{                                                                        \
		*ppvObject = (Interface *)this;                                      \
	}

#define COM_QUERY_INF_CHECK_IID(Interface) \
	if (IsEqualIID(IID_##Interface, riid)) \
	{                                      \
		*ppvObject = (Interface *)this;    \
	}

#define COM_QUERY_INF_END \
	if (*ppvObject)       \
	{                     \
		AddRef();         \
		return S_OK;      \
	}                     \
	return E_NOINTERFACE;

#define COM_REF_DECLAR                                                   \
	ULONG __stdcall AddRef(void) override { return COMBase::AddRef(); }; \
	ULONG __stdcall Release(void) override { return COMBase::Release(); };

	class COMObjectBase : protected IUnknown
	{
	protected:
		ULONG __stdcall AddRef(void) override
		{
			return ++m_ulRef;
		};
		ULONG __stdcall Release(void) override
		{
			if (--m_ulRef <= 0)
			{
				delete this;
				return 0;
			}
			return m_ulRef;
		};

	private:
		ULONG m_ulRef = 0;
	};
}
