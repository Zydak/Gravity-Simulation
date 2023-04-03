#include "application.h"

#include "objects/planet.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

static void check_vk_result(VkResult err)
{
    if (err == 0)
        return;
    fprintf(stderr, "[vulkan] Error: VkResult = %d\n", err);
    if (err < 0)
        abort();
}

struct GlobalUbo
{
    glm::mat4 projection{1.0f};
    glm::mat4 view{1.0f};
    glm::vec3 lightPosition{0.0f};
    alignas(16) glm::vec4 lightColor{1.0f, 1.0f, 1.0f, 1.0f}; // w is light intesity
};

Application::Application()
{
    m_GlobalPool = DescriptorPool::Builder(m_Device)
        .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2)
        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .Build();
    
    LoadGameObjects();
}

Application::~Application()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
void Application::Run()
{
    ImGui::CreateContext();
    auto io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

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
        .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
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
    ImGui_ImplGlfw_InitForVulkan(m_Window.GetGLFWwindow(), true);
    ImGui_ImplVulkan_InitInfo info;
    info.Instance = m_Device.GetInstance();
    info.PhysicalDevice = m_Device.GetPhysicalDevice();
    info.Device = m_Device.GetDevice();
    info.Queue = m_Device.GetGraphicsQueue();
    info.DescriptorPool = m_GlobalPool->GetDescriptorPool();
    info.Subpass = 0;
    info.MinImageCount = 2;
    info.ImageCount = m_Renderer.GetSwapChainImageCount();
    info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&info, m_Renderer.GetSwapChainRenderPass());
 
    VkCommandBuffer cmdBuffer;
    m_Device.BeginSingleTimeCommands(cmdBuffer);
    ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
    m_Device.EndSingleTimeCommands(cmdBuffer);
 
    vkDeviceWaitIdle(m_Device.GetDevice());
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();

        auto newTime = std::chrono::high_resolution_clock::now();
        float frameTime = std::chrono::duration<float, std::chrono::seconds::period>(newTime - currentTime).count();
        currentTime = newTime;
        //std::cout << frameTime * 3600 << std::endl;

        if (!ImGui::GetIO().WantCaptureMouse)
        {
            m_CameraController.Update(frameTime, m_Camera);
        }
        

        m_Camera.SetViewTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        float aspectRatio = m_Renderer.GetAspectRatio();
        m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 1000.0f);

        if (auto commandBuffer = m_Renderer.BeginFrame({0.02f, 0.02f, 0.02f})) 
        {
            int frameIndex = m_Renderer.GetFrameIndex();

            GlobalUbo ubo{};
            ubo.projection = m_Camera.GetProjection();
            ubo.view = m_Camera.GetView();
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

            FrameInfo frameInfo{};
            frameInfo.frameIndex = frameIndex;
            frameInfo.camera = &m_Camera;
            frameInfo.frameTime = frameTime;
            frameInfo.commandBuffer = commandBuffer;
            frameInfo.globalDescriptorSet = globalDescriptorSets[frameIndex];
            frameInfo.gameObjects = m_GameObjects;

            Update(frameInfo.frameTime, 50);
            m_Renderer.RenderGameObjects(frameInfo);

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
 
            ImGui::ShowDemoWindow();
 
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

            m_Renderer.EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void Application::LoadGameObjects()
{
    Properties properties1{};
    properties1.velocity = {0.0f, 0.0f, 5.0f};
    properties1.mass = 5;
    properties1.isStatic = false;

    Transform transform1{};
    transform1.translation = {10.0f, 0.0f, 0.0f};
    transform1.scale = {0.5f, 0.5f, 0.5f};
    transform1.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj1 = std::make_unique<Planet>(m_Device, "assets/models/sphere.obj", transform1, properties1);
    m_GameObjects.emplace(obj1->GetObjectID(), std::move(obj1));

    Properties properties2{};
    properties2.velocity = {0.0f, 0.0f, 0.0f};
    properties2.mass = 50;
    properties2.isStatic = true;

    Transform transform2{};
    transform2.translation = {0.0f, 0.0f, 0.0f};
    transform2.scale = {0.5f, 0.5f, 0.5f};
    transform2.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj2 = std::make_unique<Planet>(m_Device, "assets/models/sphere.obj", transform2, properties2);
    m_GameObjects.emplace(obj2->GetObjectID(), std::move(obj2));
}

void Application::Update(float delta, uint32_t substeps)
{
    for (auto& kv: m_GameObjects)
    {
        auto& obj = kv.second;
        const float stepDelta = delta / substeps;
        for (int i = 0; i < substeps; i++)
        {
            obj->Update(m_GameObjects, stepDelta);
        }
    }
}