#pragma once
#ifndef _TEXTEDITSINK_H_
#define _TEXTEDITSINK_H_

#include <boost/signals2.hpp>
#include "COMBase.h"
#include "Context.h"
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
#endif // !_TEXTEDITSINK_H_
