#include "application.h"

Application::Application()
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