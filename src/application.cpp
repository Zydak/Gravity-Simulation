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
    auto currentTime = std::chrono::high_resolution_clock::now();

    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;

        m_CameraController.Update(m_Camera);

        m_Camera.SetViewTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        float aspectRatio = m_Renderer.GetAspectRatio();
        m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 100.0f);

        if (auto commandBuffer = m_Renderer.BeginFrame()) 
        {
            m_Renderer.RenderGameObjects(commandBuffer, m_GameObjects, m_Camera);
            m_Renderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void Application::LoadGameObjects()
{
    Properties properties{};
    // Empty for now
    
    Transform transform1{};
    transform1.translation = {0.0f, 0.0f, 0.0f};
    transform1.scale = {1.0f, 1.0f, 1.0f};
    transform1.rotation = {0.0f, 0.0f, 0.0f};

    Transform transform2{};
    transform2.translation = {5.0f, 0.0f, 0.0f};
    transform2.scale = {1.0f, 1.0f, 1.0f};
    transform2.rotation = {0.0f, 0.0f, 0.0f};

    std::unique_ptr<Object> obj1 = std::make_unique<Triangle>(m_Device, "assets/models/smooth_vase.obj", transform1, properties);
    std::unique_ptr<Object> obj2 = std::make_unique<Triangle>(m_Device, "assets/models/colored_cube.obj", transform2, properties);

    m_GameObjects.push_back(std::move(obj1));
    m_GameObjects.push_back(std::move(obj2));
}