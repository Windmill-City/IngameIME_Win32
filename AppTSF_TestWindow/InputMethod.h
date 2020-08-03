#pragma once
#include <thread>

#include "../libtf/tf_application.h"
#include "../libtf/Document.h"
#include "../libtf/Context.h"
class InputMethod
{
public:
	InputMethod();
	~InputMethod();

	VOID Initialize();

	//TextServiceFramework
	Application* app;
	Document* doc;
	Context* context;
};
