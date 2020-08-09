#include "InputMethod.h"

InputMethod::InputMethod()
{
	app = new Application();
	HRESULT hr = app->Initialize();
	if (FAILED(hr))
		throw "Failed to Initialize Application:" + hr;
}

InputMethod::~InputMethod()
{
	if (context) delete context;
	if (doc) delete doc;
	app->m_pThreadMgr->Deactivate();
	if (app) delete app;
}

VOID InputMethod::Initialize()
{
	doc = new Document(app, NULL);
	context = new Context(doc);
	doc->m_pDocMgr->Push(context->m_pCtx);
}