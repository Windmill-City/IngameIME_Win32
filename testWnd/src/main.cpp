#include <iostream>
#include <string>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#include "InputWindow.hpp"

void error_callback(int error, const char* description)
{
    std::cout << "Error:" << error << description << std::endl;
}

int main()
{
    glfwSetErrorCallback(error_callback);
    if (glfwInit()) {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Set locale to the system active for wprintf
        setlocale(LC_ALL, "");

        try {
            auto window = std::make_unique<InputWindow>();
            window->runEventLoop();
        }
        catch (const std::exception& e) {
            std::cerr << e.what() << '\n';
        }

        glfwTerminate();
        return 0;
    }
    return -1;
}