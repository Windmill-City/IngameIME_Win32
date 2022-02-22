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

class InputWindow {
  public:
    static std::map<GLFWwindow*, InputWindow*> WindowMap;

  public:
    int                                      m_Width;
    int                                      m_Height;
    GLFWwindow*                              m_Window;
    std::shared_ptr<IngameIME::InputContext> m_InputContext;
    bool                                     m_FullScreen = false;

  public:
    static InputWindow* getByGLFWwindow(GLFWwindow* window)
    {
        return WindowMap[window];
    }

  public:
    InputWindow()
    {
        m_Window            = glfwCreateWindow(m_Width = 800, m_Height = 600, "libtf", NULL, NULL);
        WindowMap[m_Window] = this;

        if (!m_Window) throw std::exception("Failed to create window");

        glfwMakeContextCurrent(m_Window);
        glfwSetKeyCallback(m_Window, key_callback);
        glfwSetWindowSizeCallback(m_Window, window_size_callback);
        glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw std::exception("Failed to initialize GLAD");

        // Create InputContext
        m_InputContext = IngameIME::Global::getInstance().getInputContext(glfwGetWin32Window(m_Window));

        // Register callbacks
        m_InputContext->comp->IngameIME::PreEditCallbackHolder::setCallback([this](auto&& state, auto&& ctx) {
            switch (state) {
                case IngameIME::CompositionState::Begin: wprintf(L"Composition Begin!\n"); break;
                case IngameIME::CompositionState::Update:
                    wprintf(L"Composition Update!\n");
                    wprintf(L"PreEdit: %S\n", ctx->content.c_str());
                    wprintf(L"Sel: %d-%d\n", ctx->selStart, ctx->selEnd);
                    break;
                case IngameIME::CompositionState::End: wprintf(L"Composition End\n"); break;
            }
        });

        m_InputContext->comp->IngameIME::CommitCallbackHolder::setCallback([this](auto&& commit) {
            // Show commit
            wprintf(L"Commit: %S\n", commit.c_str());
        });

        m_InputContext->comp->IngameIME::PreEditRectCallbackHolder::setCallback([this](auto&& rect) {
            rect.left   = 20;
            rect.top    = 20;
            rect.right  = 20;
            rect.bottom = 20;
        });

        m_InputContext->comp->IngameIME::CandidateListCallbackHolder::setCallback([this](auto&& state, auto&& ctx) {
            switch (state) {
                case IngameIME::CandidateListState::Begin: wprintf(L"CandidateList Begin!\n"); break;
                case IngameIME::CandidateListState::Update:
                    wprintf(L"CandidateList Update!\n");
                    wprintf(L"Selection: %d\n", ctx->selection);
                    for (auto&& cand : ctx->candidates) { wprintf(L"%S\n", cand.c_str()); }
                    break;
                case IngameIME::CandidateListState::End: wprintf(L"CandidateList End!\n"); break;
            }
        });

        m_InputContext->setActivated(true);
        m_InputContext->setFullScreen(true);
    }

  public:
    void runEventLoop()
    {
        while (!glfwWindowShouldClose(m_Window)) {
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
            glfwSwapBuffers(m_Window);
            glfwPollEvents();
        }
    }
    void switchFullScreen()
    {
        m_FullScreen = !m_FullScreen;
        // Update FullScreen state
        m_InputContext->setFullScreen(m_FullScreen);

        wprintf(L"FullScreen:%wS\n", m_FullScreen ? L"True" : L"False");

        auto monitor = glfwGetPrimaryMonitor();
        int  monitorWidth, monitorHeight;
        glfwGetMonitorWorkarea(monitor, NULL, NULL, &monitorWidth, &monitorHeight);

        if (m_FullScreen) {
            glfwSetWindowMonitor(m_Window, monitor, 0, 0, monitorWidth, monitorHeight, GLFW_DONT_CARE);
        }
        else
            glfwSetWindowMonitor(m_Window,
                                 NULL,
                                 (monitorWidth - m_Width) / 2,
                                 (monitorHeight - m_Height) / 2,
                                 m_Width,
                                 m_Height,
                                 GLFW_DONT_CARE);
    }
};

std::map<GLFWwindow*, InputWindow*> InputWindow::WindowMap = std::map<GLFWwindow*, InputWindow*>();

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    // Close Window
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) { glfwSetWindowShouldClose(window, GLFW_TRUE); }
    // Switch Fullscreen
    if (glfwGetKey(window, GLFW_KEY_F11) == GLFW_PRESS) { InputWindow::getByGLFWwindow(window)->switchFullScreen(); }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    auto inputWindow = InputWindow::getByGLFWwindow(window);
    if (!inputWindow->m_FullScreen) {
        inputWindow->m_Width  = width;
        inputWindow->m_Height = height;
    }
}
