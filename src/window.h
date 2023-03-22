#pragma once

#define GLFW_INCLUDE_VULKAN
#include "GLFW/glfw3.h"
#include <string>

class Window
{
public:
    Window(int width, int height, std::string name);
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void CreateWindowSurface(VkInstance instance, VkSurfaceKHR* surface);

    inline bool ShouldClose() { return glfwWindowShouldClose(m_Window); }

private:
    const int m_Width;
    const int m_Height;
    std::string m_Name;

    GLFWwindow *m_Window;
};