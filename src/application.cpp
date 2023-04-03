#include "application.h"

#include "objects/triangle.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

struct GlobalUbo
{
    alignas(16) glm::mat4 projectionView{1.0f};
    alignas(16) glm::vec3 lightDirection = glm::normalize(glm::vec3{1.0f, -3.0f, -1.0f});
};

Application::Application()
{
    m_GlobalPool = DescriptorPool::Builder(m_Device)
        .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .Build();
    
    LoadGameObjects();
}

Application::~Application()
{
    
}
void Application::Run()
{
    auto currentTime = std::chrono::high_resolution_clock::now();

    std::vector<std::unique_ptr<Buffer>> uboBuffers(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < uboBuffers.size(); i++)
    {
        uboBuffers[i] = std::make_unique<Buffer>(
            m_Device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffers[i]->Map();
    }

    auto globalSetLayout = DescriptorSetLayout::Builder(m_Device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
        .Build();

    Renderer m_Renderer(m_Window, m_Device, globalSetLayout->GetDescriptorSetLayout());

    std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++)
    {
        auto bufferInfo = uboBuffers[i]->DescriptorInfo();
        DescriptorWriter(*globalSetLayout, *m_GlobalPool)
            .WriteBuffer(0, &bufferInfo)
            .Build(globalDescriptorSets[i]);
    }

    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        std::cout << frameTime * 3600 << std::endl;

        m_CameraController.Update(frameTime, m_Camera);

        m_Camera.SetViewTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        float aspectRatio = m_Renderer.GetAspectRatio();
        m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 100.0f);

        if (auto commandBuffer = m_Renderer.BeginFrame()) 
        {
            int frameIndex = m_Renderer.GetFrameIndex();

            GlobalUbo ubo{};
            ubo.projectionView = m_Camera.GetProjection() * m_Camera.GetView();
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

            FrameInfo frameInfo{};
            frameInfo.frameIndex = frameIndex;
            frameInfo.camera = &m_Camera;
            frameInfo.frameTime = frameTime;
            frameInfo.commandBuffer = commandBuffer;
            frameInfo.globalDescriptorSet = globalDescriptorSets[frameIndex];

            m_Renderer.RenderGameObjects(frameInfo, m_GameObjects);
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
    transform1.translation = {0.0f, 0.5f, 0.0f};
    transform1.scale = {3.0f, 3.0f, 3.0f};
    transform1.rotation = {0.0f, 0.0f, 0.0f};

    Transform transform2{};
    transform2.translation = {1.0f, 0.5f, 0.0f};
    transform2.scale = {3.0f, 3.0f, 3.0f};
    transform2.rotation = {0.0f, 0.0f, 0.0f};

    std::unique_ptr<Object> obj1 = std::make_unique<Triangle>(m_Device, "assets/models/flat_vase.obj", transform1, properties);
    std::unique_ptr<Object> obj2 = std::make_unique<Triangle>(m_Device, "assets/models/smooth_vase.obj", transform2, properties);

    m_GameObjects.push_back(std::move(obj1));
    m_GameObjects.push_back(std::move(obj2));
}