#include "InputWindow.hpp"

std::map<GLFWwindow*, InputWindow*> InputWindow::WindowMap = std::map<GLFWwindow*, InputWindow*>();

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    // Switch Fullscreen
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS)
    {
        InputWindow::getByGLFWwindow(window)->switchFullScreen();
    }
    // Switch Activate
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS)
    {
        auto inputCtx = InputWindow::getByGLFWwindow(window)->inputCtx;
        inputCtx->setActivated(!inputCtx->getActivated());
        printf("Activated:%s\n", inputCtx->getActivated() ? "True" : "False");
    }
    // Activate Initial InputProc
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS)
    {
        InputWindow::getByGLFWwindow(window)->initialProc->setActivated();
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    auto inputWindow = InputWindow::getByGLFWwindow(window);
    if (!inputWindow->fullscreen)
    {
        inputWindow->width  = width;
        inputWindow->height = height;
    }
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void debugShowInputProcessor(std::shared_ptr<const IngameIME::InputProcessor> proc)
{
    printf("[%s][%ls][%ls]: %ls\n",
           proc->type == IngameIME::InputProcessorType::KeyboardLayout ? "KL" : "TIP",
           proc->locale->locale.c_str(),
           proc->locale->name.c_str(),
           proc->name.c_str());
}

InputWindow* InputWindow::getByGLFWwindow(GLFWwindow* window)
{
    return WindowMap[window];
}

InputWindow::InputWindow()
{
    window            = glfwCreateWindow(width = 800, height = 600, "libtf", NULL, NULL);
    WindowMap[window] = this;

    if (!window) throw std::runtime_error("Failed to create window");

    glfwMakeContextCurrent(window);
    glfwSetKeyCallback(window, key_callback);
    glfwSetWindowSizeCallback(window, window_size_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw std::runtime_error("Failed to initialize GLAD");

    // Print library version
    printf("IngameIME Library Version: %s\n", IngameIME::Global::Version.c_str());

    // Create InputContext
    inputCtx = IngameIME::Global::getInstance().getInputContext(glfwGetWin32Window(window));

    // Register callbacks
    inputCtx->comp->IngameIME::PreEditCallbackHolder::setCallback(
        [this](auto&& state, auto&& ctx)
        {
            switch (state)
            {
            case IngameIME::CompositionState::Begin:
                printf("Composition Begin!\n");
                break;
            case IngameIME::CompositionState::Update:
                printf("Composition Update!\n");
                printf("PreEdit: %ls\n", ctx->content.c_str());
                printf("Sel: %d-%d\n", ctx->selStart, ctx->selEnd);
                break;
            case IngameIME::CompositionState::End:
                printf("Composition End\n");
                break;
            }
        });

    inputCtx->comp->IngameIME::CommitCallbackHolder::setCallback(
        [this](auto&& commit)
        {
            // Show commit
            printf("Commit: %ls\n", commit.c_str());
        });

    inputCtx->comp->IngameIME::PreEditRectCallbackHolder::setCallback(
        [this](auto&& rect)
        {
            rect.left   = 20;
            rect.top    = 20;
            rect.right  = 20;
            rect.bottom = 20;
            printf("Update PreEditRect\n");
        });

    inputCtx->comp->IngameIME::CandidateListCallbackHolder::setCallback(
        [this](auto&& state, auto&& ctx)
        {
            switch (state)
            {
            case IngameIME::CandidateListState::Begin:
                printf("CandidateList Begin!\n");
                break;
            case IngameIME::CandidateListState::Update:
                printf("CandidateList Update!\n");
                printf("Selection: %d\n", ctx->selection);
                for (auto&& cand : ctx->candidates)
                {
                    printf("%ls\n", cand.c_str());
                }
                break;
            case IngameIME::CandidateListState::End:
                printf("CandidateList End!\n");
                break;
            }
        });

    IngameIME::Global::getInstance().setCallback(
        [this](auto&& state, auto&& ctx)
        {
            switch (state)
            {
            case IngameIME::InputProcessorState::FullUpdate:
                printf("InputProcessor Full Update!\n");
                break;
            case IngameIME::InputProcessorState::InputModeUpdate:
                printf("InputProcessor InputMode Update!\n");
                break;
            }

            debugShowInputProcessor(ctx.proc);

            printf("InputModes:\n");
            for (auto&& mode : ctx.modes)
            {
                printf("%d\n", mode);
            }
        });

    printf("Active InputProc:\n");
    initialProc = IngameIME::Global::getInstance().getActiveInputProcessor();
    debugShowInputProcessor(initialProc);

    printf("InputProcs:\n");
    for (auto proc : IngameIME::Global::getInstance().getInputProcessors())
    {
        debugShowInputProcessor(proc);
    }

    printf("Press F3 to activate the InputProcessor recored when inital\n");
    printf("Press F5 to switch on/off InputContext\n");
    printf("Press F11 to switch on/off fullscreen\n");
}

bool InputWindow::pollEvent()
{
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    glfwSwapBuffers(window);
    glfwPollEvents();

    return !glfwWindowShouldClose(window);
}

void InputWindow::switchFullScreen()
{
    fullscreen = !fullscreen;
    // Update FullScreen state
    inputCtx->setFullScreen(fullscreen);

    printf("FullScreen:%s\n", fullscreen ? "True" : "False");

    auto monitor = glfwGetPrimaryMonitor();
    int  monitorWidth, monitorHeight;
    glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitorWidth, &monitorHeight);
}
