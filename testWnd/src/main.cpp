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
    if (glfwInit())
    {
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        // Must call once to set active locale
        setlocale(LC_ALL, "");

        try
        {
            auto window = std::make_unique<InputWindow>();
            while (window->pollEvent())
                ;
        }
        catch (std::exception& e)
        {
            std::cerr << e.what() << '\n';
        }
        catch (...)
        {
            std::cerr << "err occurs!" << '\n';
        }

        glfwTerminate();
        return 0;
    }
    return -1;
}