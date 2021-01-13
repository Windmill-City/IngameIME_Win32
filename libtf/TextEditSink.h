#pragma once
#include <functional>
#include "COMBase.h"
#include "Context.h"
namespace libtf {
	class TFAPI TextEditSink :
		public COMBase,
		public ITfTextEditSink
	{
	private:
		Context* m_Ctx;
		DWORD					m_Cookie = TF_INVALID_COOKIE;
	public:
		TextEditSink(Context* context);
		~TextEditSink();

		//ITfTextEditSink
		virtual HRESULT __stdcall OnEndEdit(ITfContext* pic, TfEditCookie ecReadOnly, ITfEditRecord* pEditRecord) override;

		//COMBase
		virtual HRESULT __stdcall QueryInterface(REFIID riid, void** ppvObject) override {
			TF_COM_ASUNK(ITfTextEditSink);
			TF_COM_RETURN;
		}
		TF_COM_REFS;
	};
}
