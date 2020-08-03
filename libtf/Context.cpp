#include "pch.h"
#include "Context.h"

Context::Context(Document* doc, IUnknown* punk)
{
	textStore = punk;
	document = doc;

	document->docMgr->CreateContext(document->common->client_id, 0, textStore, &context, &editCookie);
	if (textStore)
		textStore->AddRef();
}

Context::~Context()
{
	if (context) {
		context->Release();
		context = NULL;
	}
	if (textStore) {
		textStore->Release();
		textStore = NULL;
	}
}

VOID Context::push()
{
	ITfContext* top;
	document->docMgr->GetTop(&top);
	if (top != context) {
		document->docMgr->Pop(TF_POPF_ALL);
		document->docMgr->Push(context);
	}
}