#include "application.h"

#include "objects/triangle.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

Application::Application()
{
    LoadGameObjects();
}

Application::~Application()
{
    
}
void Application::Run()
{
    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();
        if (auto commandBuffer = m_Renderer.BeginFrame()) 
        {
            m_Renderer.RenderGameObjects(commandBuffer, m_GameObjects);
            m_Renderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void Application::LoadGameObjects()
{
    std::vector<Model::Vertex> vertices
    {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    Properties properties{};
    // Empty for now
    
    Transform2d transform{};
    transform.translation.x = 0.0f;
    transform.translation.y = 0.0f;
    transform.scale = {1.0f, 1.0f};
    transform.rotation = 0.25f * glm::two_pi<float>();
    //transform.rotation = 0;

    std::unique_ptr<Object> obj = std::make_unique<Triangle>(m_Device, vertices, transform, properties);

    m_GameObjects.push_back(std::move(obj));
}