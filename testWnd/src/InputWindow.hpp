#pragma once
#include <exception>
#include <map>
#include <memory>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "IngameIME.hpp"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_size_callback(GLFWwindow* window, int width, int height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

void debugShowInputProcessor(std::shared_ptr<const IngameIME::InputProcessor> proc)
{
    wprintf(L"[%S][%wS][%wS]: %wS\n",
            proc->type == IngameIME::InputProcessorType::KeyboardLayout ? "KL" : "TIP",
            proc->locale->locale.c_str(),
            proc->locale->name.c_str(),
            proc->name.c_str());
}

class InputWindow {
  public:
    static std::map<GLFWwindow*, InputWindow*> WindowMap;

  public:
    int                                      width;
    int                                      height;
    GLFWwindow*                              window;
    std::shared_ptr<IngameIME::InputContext> inputCtx;
    bool                                     fullscreen = false;

    std::shared_ptr<const IngameIME::InputProcessor> initialProc;

  public:
    static InputWindow* getByGLFWwindow(GLFWwindow* window)
    {
        return WindowMap[window];
    }

  public:
    InputWindow()
    {
        window            = glfwCreateWindow(width = 800, height = 600, "libtf", NULL, NULL);
        WindowMap[window] = this;

        if (!window) throw std::exception("Failed to create window");

        glfwMakeContextCurrent(window);
        glfwSetKeyCallback(window, key_callback);
        glfwSetWindowSizeCallback(window, window_size_callback);
        glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw std::exception("Failed to initialize GLAD");

        // Create InputContext
        inputCtx = IngameIME::Global::getInstance().getInputContext(glfwGetWin32Window(window));

        // Register callbacks
        inputCtx->comp->IngameIME::PreEditCallbackHolder::setCallback([this](auto&& state, auto&& ctx) {
            switch (state) {
                case IngameIME::CompositionState::Begin: wprintf(L"Composition Begin!\n"); break;
                case IngameIME::CompositionState::Update:
                    wprintf(L"Composition Update!\n");
                    wprintf(L"PreEdit: %wS\n", ctx->content.c_str());
                    wprintf(L"Sel: %d-%d\n", ctx->selStart, ctx->selEnd);
                    break;
                case IngameIME::CompositionState::End: wprintf(L"Composition End\n"); break;
            }
        });

        inputCtx->comp->IngameIME::CommitCallbackHolder::setCallback([this](auto&& commit) {
            // Show commit
            wprintf(L"Commit: %wS\n", commit.c_str());
        });

        inputCtx->comp->IngameIME::PreEditRectCallbackHolder::setCallback([this](auto&& rect) {
            rect.left   = 20;
            rect.top    = 20;
            rect.right  = 20;
            rect.bottom = 20;
            wprintf(L"Update PreEditRect\n");
        });

        inputCtx->comp->IngameIME::CandidateListCallbackHolder::setCallback([this](auto&& state, auto&& ctx) {
            switch (state) {
                case IngameIME::CandidateListState::Begin: wprintf(L"CandidateList Begin!\n"); break;
                case IngameIME::CandidateListState::Update:
                    wprintf(L"CandidateList Update!\n");
                    wprintf(L"Selection: %d\n", ctx->selection);
                    for (auto&& cand : ctx->candidates) { wprintf(L"%wS\n", cand.c_str()); }
                    break;
                case IngameIME::CandidateListState::End: wprintf(L"CandidateList End!\n"); break;
            }
        });

        IngameIME::Global::getInstance().setCallback([this](auto&& state, auto&& ctx) {
            switch (state) {
                case IngameIME::InputProcessorState::FullUpdate: wprintf(L"InputProcessor Full Update!\n"); break;
                case IngameIME::InputProcessorState::InputModeUpdate:
                    wprintf(L"InputProcessor InputMode Update!\n");
                    break;
            }

            debugShowInputProcessor(ctx.proc);

            wprintf(L"InputModes:\n");
            for (auto&& mode : ctx.modes) { wprintf(L"%wS\n", mode.c_str()); }
        });

        wprintf(L"Active InputProc:\n");
        initialProc = IngameIME::Global::getInstance().getActiveInputProcessor();
        debugShowInputProcessor(initialProc);

        wprintf(L"InputProcs:\n");
        for (auto proc : IngameIME::Global::getInstance().getInputProcessors()) { debugShowInputProcessor(proc); }
    }

  public:
    void runEventLoop()
    {
        while (!glfwWindowShouldClose(window)) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }
    }
    void switchFullScreen()
    {
        fullscreen = !fullscreen;
        // Update FullScreen state
        inputCtx->setFullScreen(fullscreen);

        wprintf(L"FullScreen:%wS\n", fullscreen ? L"True" : L"False");

        auto monitor = glfwGetPrimaryMonitor();
        int  monitorWidth, monitorHeight;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitorWidth, &monitorHeight);

        // if (fullscreen) { glfwSetWindowMonitor(window, monitor, 0, 0, monitorWidth, monitorHeight, GLFW_DONT_CARE); }
        // else
        //     glfwSetWindowMonitor(
        //         window, NULL, (monitorWidth - width) / 2, (monitorHeight - height) / 2, width, height,
        //         GLFW_DONT_CARE);
    }
};

std::map<GLFWwindow*, InputWindow*> InputWindow::WindowMap = std::map<GLFWwindow*, InputWindow*>();

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
    // Switch Fullscreen
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) { InputWindow::getByGLFWwindow(window)->switchFullScreen(); }
    // Switch Activate
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS) {
        auto inputCtx = InputWindow::getByGLFWwindow(window)->inputCtx;
        inputCtx->setActivated(!inputCtx->getActivated());
        wprintf(L"Activated:%wS\n", inputCtx->getActivated() ? L"True" : L"False");
    }
    // Activate Initial InputProc
    if (glfwGetKey(window, GLFW_KEY_F3) == GLFW_PRESS) {
        InputWindow::getByGLFWwindow(window)->initialProc->setActivated();
    }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    auto inputWindow = InputWindow::getByGLFWwindow(window);
    if (!inputWindow->fullscreen) {
        inputWindow->width  = width;
        inputWindow->height = height;
    }
}
