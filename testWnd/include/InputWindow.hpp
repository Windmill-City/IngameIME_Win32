#pragma once
#include <map>
#include <memory>
#include <stdexcept>

#include <glad/glad.h>
#include <glfw/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include "IngameIME.hpp"

class InputWindow
{
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
    static InputWindow* getByGLFWwindow(GLFWwindow* window);

  public:
    InputWindow();

  public:
    bool pollEvent();
    void switchFullScreen();
};
