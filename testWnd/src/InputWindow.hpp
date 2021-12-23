#pragma once
#include <exception>
#include <map>
#include <memory>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "libtf.h"

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void window_size_callback(GLFWwindow* window, int width, int height);
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    glViewport(0, 0, width, height);
}

/**
 * @brief Receive candidate list data in fullscreen mode
 *
 * @param userData we pass window here when install callback
 */
void process_candidate_list(const libtf_CandidateListState_t  state,
                            const libtf_pCandidateListContext ctx,
                            void*                             userData);
/**
 * @brief Receive preedit and compostion state
 *
 * @param userData we pass window here when install callback
 */
void process_preedit(const libtf_CompositionState_t state, const libtf_pPreEditContext ctx, void* userData);
/**
 * @brief Return the boundary rectangle of the preedit, in window coordinate
 *
 * @param userData we pass window here when install callback
 *
 * @note If the length of preedit is 0 (as it would be drawn by input method), the rectangle
 * coincides with the insertion point, and its width is 0.
 */
void process_preedit_rect(RECT* rect, void* userData);
/**
 * @brief Receives commit string from input method
 *
 * @param userData we pass window here when install callback
 */
void process_commit(const wchar_t* commit, void* userData);
/**
 * @brief Receive InputProcessor relevent event
 *
 * @param userData we pass window here when install callback
 */
void process_inputprocessor(const libtf_InputProcessorState_t  state,
                            const libtf_pInputProcessorContext ctx,
                            void*                              userData);
/**
 * @brief Print all the inputprocessors available
 */
void show_inputprocessors();

class InputWindow {
  public:
    static std::map<GLFWwindow*, InputWindow*> WindowMap;

  public:
    int                 m_Width;
    int                 m_Height;
    GLFWwindow*         m_Window;
    libtf_pInputContext m_InputContext;
    bool                m_FullScreen = false;

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

        if (!m_Window) throw new std::exception("Failed to create window");

        glfwMakeContextCurrent(m_Window);
        glfwSetKeyCallback(m_Window, key_callback);
        glfwSetWindowSizeCallback(m_Window, window_size_callback);
        glfwSetFramebufferSizeCallback(m_Window, framebuffer_size_callback);

        HRESULT hr;
        BEGIN_HRESULT_SCOPE();

        show_inputprocessors();

        // Create InputContext
        CHECK_HR(libtf_create_ctx(&m_InputContext, glfwGetWin32Window(m_Window)));

        // Install callbacks
        libtf_candidate_list_set_callback(m_InputContext, process_candidate_list, this);
        libtf_preedit_set_callback(m_InputContext, process_preedit, this);
        libtf_preedit_rect_set_callback(m_InputContext, process_preedit_rect, this);
        libtf_commit_set_callback(m_InputContext, process_commit, this);
        libtf_inputprocessor_set_callback(m_InputContext, process_inputprocessor, this);

        CHECK_HR(libtf_set_activated(m_InputContext, true));

        // glad: load all OpenGL function pointers
        if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) throw new std::exception("Failed to initialize GLAD");

        return;
        END_HRESULT_SCOPE();
        throw new std::exception("Failed to initialize InputContext:" + hr);
    }
    ~InputWindow()
    {
        HRESULT hr;
        BEGIN_HRESULT_SCOPE();

        // Detroy InputContext
        CHECK_HR(libtf_destroy_ctx(&m_InputContext));

        END_HRESULT_SCOPE();
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
        libtf_set_fullscreen(m_InputContext, m_FullScreen);

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
    // Show InputProcessors
    if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS) { show_inputprocessors(); }
}

void window_size_callback(GLFWwindow* window, int width, int height)
{
    auto inputWindow = InputWindow::getByGLFWwindow(window);
    if (!inputWindow->m_FullScreen) {
        inputWindow->m_Width  = width;
        inputWindow->m_Height = height;
    }
}

/**
 * @brief Receive candidate list data in fullscreen mode
 *
 * @param userData we pass window here when install callback
 */
void process_candidate_list(const libtf_CandidateListState_t  state,
                            const libtf_pCandidateListContext ctx,
                            void*                             userData)
{
    if (state == libtf_CandidateListBegin) { wprintf(L"CandidateList Begin!\n"); }
    else if (state == libtf_CandidateListUpdate) {
        wprintf(L"CandidateList Update!\n");
        wprintf(L"Sel:%d\n", ctx->m_Selection);
        for (size_t i = 0; i < ctx->m_PageSize; i++) { wprintf(L"[%zd]%wS\n", i, ctx->m_Candidates[i]); }
    }
    else {
        wprintf(L"CandidateList End!\n");
    }
}
/**
 * @brief Receive preedit and compostion state
 *
 * @param userData we pass window here when install callback
 */
void process_preedit(const libtf_CompositionState_t state, const libtf_pPreEditContext ctx, void* userData)
{
    if (state == libtf_CompositionBegin) { wprintf(L"Composition Begin!\n"); }
    else if (state == libtf_CompositionUpdate) {
        wprintf(L"Composition Update!\n");
        wprintf(L"PreEdit:%wS, Sel:<%d,%d>\n", ctx->m_Content, ctx->m_SelStart, ctx->m_SelEnd);
    }
    else {
        wprintf(L"Composition End!\n");
    }
}
/**
 * @brief Return the boundary rectangle of the preedit, in window coordinate
 *
 * @param userData we pass window here when install callback
 *
 * @note If the length of preedit is 0 (as it would be drawn by input method), the rectangle
 * coincides with the insertion point, and its width is 0.
 */
void process_preedit_rect(RECT* rect, void* userData)
{
    rect->bottom = 8;
}
/**
 * @brief Receives commit string from input method
 *
 * @param userData we pass window here when install callback
 */
void process_commit(const wchar_t* commit, void* userData)
{
    wprintf(L"Commit:%wS\n", commit);
}

void print_inputprocessor_profile(libtf_HInputProcessor processor)
{
    auto processorCtx = libtf_inputprocessor_get_profile(processor);

    wprintf(L"[%wS]Locale:%wS, LangName:%wS, InputProcessor:%wS\n",
            processorCtx->m_Type == libtf_KeyboardLayout ? L"KL" : L"TIP",
            processorCtx->m_Locale,
            processorCtx->m_LocaleName,
            processorCtx->m_InputProcessorName);

    libtf_inputprocessor_free_profile(&processorCtx);
}

/**
 * @brief Receive InputProcessor relevent event
 *
 * @param userData we pass window here when install callback
 */
void process_inputprocessor(const libtf_InputProcessorState_t  state,
                            const libtf_pInputProcessorContext ctx,
                            void*                              userData)
{
    if (state == libtf_InputProcessorFullUpdate) {
        wprintf(L"InputProcessor Full Update!\n");
        print_inputprocessor_profile(ctx->m_InputProcessor);
    }
    wprintf(L"InputModes:\n");
    for (size_t i = 0; i < ctx->m_InputModeSize; i++) { wprintf(L"%wS\n", ctx->m_InputModes[i]); }
}

/**
 * @brief Print all the inputprocessors available
 */
void show_inputprocessors()
{
    auto processors = libtf_get_inputprocessors();
    wprintf(L"System available InputProcessors:\n");
    for (size_t i = 0; i < processors->m_InputProcessorsSize; i++) {
        print_inputprocessor_profile(processors->m_InputProcessors[i]);
    }
    libtf_free_inputprocessors(&processors);
}
