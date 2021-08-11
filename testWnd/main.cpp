#include "libtf.h"
#include <glfw/glfw3.h>
#include <iostream>

using namespace std;
libtf_pInputContext ctx;

void error_callback(int error, const char *description)
{
    cout << "Error:" << error << description << endl;
}

bool toggleDown = false;

void process_input(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    {
        glfwSetWindowShouldClose(window, GLFW_TRUE);
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_PRESS && !toggleDown)
    {
        toggleDown = true;
        if (FAILED(libtf_set_im_state(ctx, true)))
        {
            cout << "Failed to set im state!" << endl;
        }
        cout << "IM Enabled!" << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F6) == GLFW_PRESS && !toggleDown)
    {
        toggleDown = true;
        if (FAILED(libtf_set_im_state(ctx, false)))
        {
            cout << "Failed to set im state!" << endl;
        }
        cout << "IM Disabled!" << endl;
    }
    if (glfwGetKey(window, GLFW_KEY_F5) == GLFW_RELEASE && glfwGetKey(window, GLFW_KEY_F6) == GLFW_RELEASE)
    {
        toggleDown = false;
    }
}

void window_focus_callback(GLFWwindow *window, int focused)
{
    if (focused)
    {
        // The window gained input focus
        if (FAILED(libtf_set_focus_wnd(ctx, GetForegroundWindow())))
        {
            cout << "Failed to focus window!" << endl;
        }
    }
}

int main()
{
    glfwSetErrorCallback(error_callback);
    if (glfwInit())
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        GLFWwindow *window = glfwCreateWindow(800, 600, "libtf", NULL, NULL);
        if (!window)
        {
            cout << "Failed to create window!" << endl;
            glfwTerminate();
            return -1;
        }
        glfwMakeContextCurrent(window);

        CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
        HRESULT hr;
        if (FAILED(hr = libtf_create_ctx(&ctx)))
        {
            cout << "Failed to create input context!" << hr << endl;
            glfwTerminate();
            return -1;
        }

        glfwSetWindowFocusCallback(window, window_focus_callback);

        while (!glfwWindowShouldClose(window))
        {
            process_input(window);
            glfwSwapBuffers(window);
            glfwPollEvents();
        }

        if (FAILED(hr = libtf_dispose_ctx(ctx)))
        {
            cout << "Failed to dispose input context!" << hr << endl;
        }

        glfwTerminate();
        return 0;
    }
    return -1;
}