#pragma once

#include "GLFW/glfw3.h"
#include <string>

class Window
{
public:
    Window(int width, int height, std::string name);
    ~Window();

    inline bool ShouldClose() { return glfwWindowShouldClose(m_Window); }

private:
    const int m_Width;
    const int m_Height;
    std::string m_Name;

    GLFWwindow *m_Window;
};