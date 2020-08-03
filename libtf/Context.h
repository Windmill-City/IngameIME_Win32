#pragma once
#include "tf_common.h"
#include "Document.h"
class Context
{
public:
	Document* document;
	IUnknown* textStore;

	ITfContext* context;
	TfEditCookie editCookie;

	TF_API Context(Document* doc, IUnknown* punk);
	TF_API ~Context();
	TF_API VOID push();
private:
};
