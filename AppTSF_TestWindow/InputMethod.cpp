#include "InputMethod.h"

InputMethod::InputMethod()
{
}

InputMethod::~InputMethod()
{
	if (context) delete context;
	if (doc) delete doc;
	if (app) delete app;
}

VOID InputMethod::Initialize()
{
	app = new Application();
	HRESULT hr = app->Initialize();
	if (FAILED(hr))
		throw "Failed to Initialize Application:" + hr;
	app->Activate();
	doc = new Document(app);
	context = new Context(doc, NULL); context->push();
}