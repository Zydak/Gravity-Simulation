#include "application.h"

Application::Application()
    :   m_Window(WIDTH, HEIGHT, "Gravity"),
        m_Pipeline("shaders/shader.vert.spv", "shaders/shader.frag.spv")
{

}

Application::~Application()
{

}

void Application::Run()
{
    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();
    }
}