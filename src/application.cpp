#include "application.h"

#include "objects/planet.h"
#include "objects/orbit.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"

static float OrbitAccumulator = 0;

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

    m_Renderer = std::make_unique<Renderer>(m_Window, m_Device, globalSetLayout->GetDescriptorSetLayout());

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
    info.ImageCount = m_Renderer->GetSwapChainImageCount();
    info.CheckVkResultFn = check_vk_result;
    ImGui_ImplVulkan_Init(&info, m_Renderer->GetSwapChainRenderPass());
 
    VkCommandBuffer cmdBuffer;
    m_Device.BeginSingleTimeCommands(cmdBuffer);
    ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
    m_Device.EndSingleTimeCommands(cmdBuffer);
 
    vkDeviceWaitIdle(m_Device.GetDevice());
    ImGui_ImplVulkan_DestroyFontUploadObjects();

    auto lastUpdate = std::chrono::high_resolution_clock::now();
    float accumulator = 0;
    float FPSaccumulator = 0;
    float FPS = 0;

    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float, std::chrono::seconds::period>(now - lastUpdate).count();
        lastUpdate = now;
        accumulator += delta;
        FPSaccumulator += delta;
        OrbitAccumulator += delta;
        //std::cout << frameTime * 3600 << std::endl;

        if (!ImGui::GetIO().WantCaptureMouse)
        {
            m_CameraController.Update(delta, m_Camera);
        }

        m_Camera.SetViewTarget(glm::vec3(0.0f, 0.0f, 0.0f));

        float aspectRatio = m_Renderer->GetAspectRatio();
        m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 1000.0f);

        if (auto commandBuffer = m_Renderer->BeginFrame({0.02f, 0.02f, 0.02f})) 
        {
            int frameIndex = m_Renderer->GetFrameIndex();

            GlobalUbo ubo{};
            ubo.projection = m_Camera.GetProjection();
            ubo.view = m_Camera.GetView();
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

            FrameInfo frameInfo{};
            frameInfo.frameIndex = frameIndex;
            frameInfo.camera = &m_Camera;
            frameInfo.frameTime = delta;
            frameInfo.commandBuffer = commandBuffer;
            frameInfo.globalDescriptorSet = globalDescriptorSets[frameIndex];
            frameInfo.gameObjects = m_GameObjects;

            while (accumulator > 0.016f)
            {
                Update(frameInfo, 0.016f, 500);
                accumulator -= 0.016f;
            }
            m_Renderer->RenderGameObjects(frameInfo);
            m_Renderer->RenderLines(frameInfo, m_Lines);

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
 
            ImGui::Begin("Settings", (bool*)false, 0);
            if (FPSaccumulator > 0.5f)
            {
                FPS = 1/frameInfo.frameTime;
                FPSaccumulator -= 0.5f;
            }
            ImGui::Text("FPS %.1f", FPS);
            for (auto& kv : m_GameObjects)
            {
                auto& obj = kv.second;
                ImGui::Text("Obj ID: %d Pos: x %.0f | y %.0f, z %.0f", 
                    obj->GetObjectID(), 
                    obj->GetObjectTransform().translation.x,
                    obj->GetObjectTransform().translation.y,
                    obj->GetObjectTransform().translation.z
                );
                //ImGui::DragFloat((std::to_string(obj->GetObjectID()) + std::string(" Obj Mass")).c_str(), &obj->GetObjectProperties().mass, 0.005f);
            }
            static float x = 0;
            static float y = 0;
            static float z = 0;
            ImGui::End();
 
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

            m_Renderer->EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void Application::LoadGameObjects()
{
    Properties properties1{};
    properties1.velocity = {0.0f, 0.0f, 22.0f};
    properties1.mass = 1;
    properties1.isStatic = false;

    Transform transform1{};
    transform1.translation = {8.0f, 0.0f, 0.0f};
    transform1.scale = glm::vec3{1.0f, 1.0f, 1.0f} * properties1.mass/2.0f;
    transform1.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj1 = std::make_unique<Planet>(m_Device, "assets/models/sphere.obj", transform1, properties1);
    m_GameObjects.emplace(obj1->GetObjectID(), std::move(obj1));

    Properties properties2{};
    properties2.velocity = {0.0f, 0.0f, -15.0f};
    properties2.mass = 5;
    properties2.isStatic = false;

    Transform transform2{};
    transform2.translation = {-15.0f, 0.0f, 0.0f};
    transform2.scale = glm::vec3{1.0f, 1.0f, 1.0f} * properties2.mass/6.0f;
    transform2.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj2 = std::make_unique<Planet>(m_Device, "assets/models/sphere.obj", transform2, properties2);
    m_GameObjects.emplace(obj2->GetObjectID(), std::move(obj2));

    Properties properties3{};
    properties3.velocity = {0.0f, 0.0f, 0.0f};
    properties3.mass = 500;
    properties3.isStatic = true;

    Transform transform3{};
    transform3.translation = {0.0f, 0.0f, 0.0f};
    transform3.scale = glm::vec3{1.0f, 1.0f, 1.0f};
    transform3.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj3 = std::make_unique<Planet>(m_Device, "assets/models/sphere.obj", transform3, properties3);
    m_GameObjects.emplace(obj3->GetObjectID(), std::move(obj3));
    
    std::vector<Model::Vertex> OrbitPositions1;
    std::vector<Model::Vertex> OrbitPositions2;
    // fill array with garbage
    for (int i = 0; i < 200; i++)
    {
        OrbitPositions1.push_back({{0.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f}});
    }

    for (int i = 0; i < 200; i++)
    {
        OrbitPositions2.push_back({{0.0f, 0.0f, 0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f}});
    }
    Properties prop{};
    Transform trans{};
    std::unique_ptr<Object> orbit1 = std::make_unique<Orbit>(m_Device, OrbitPositions1, trans, prop);
    m_Lines.emplace(orbit1->GetObjectID(), std::move(orbit1));
    std::unique_ptr<Object> orbit2 = std::make_unique<Orbit>(m_Device, OrbitPositions2, trans, prop);
    m_Lines.emplace(orbit2->GetObjectID(), std::move(orbit2));
}

void Application::Update(FrameInfo frameInfo, float delta, uint32_t substeps)
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
    static std::vector<Model::Vertex> s_Positions1;
    static std::vector<Model::Vertex> s_Positions2;
    if (s_Positions1.size() >= 200)
    {
        s_Positions1.erase(s_Positions1.begin());
    }
    if (s_Positions2.size() >= 200)
    {
        s_Positions2.erase(s_Positions2.begin());
    }
    s_Positions1.push_back({m_GameObjects[0]->GetObjectTransform().translation, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f}});
    m_Lines[0]->GetObjectModel()->UpdateVertexBuffer(frameInfo.commandBuffer, m_Lines[0]->GetObjectModel()->GetVertexBuffer(), s_Positions1);
    s_Positions2.push_back({m_GameObjects[1]->GetObjectTransform().translation, {0.0f,0.0f,0.0f}, {0.0f,0.0f,0.0f}, {0.0f,0.0f}});
    m_Lines[1]->GetObjectModel()->UpdateVertexBuffer(frameInfo.commandBuffer, m_Lines[1]->GetObjectModel()->GetVertexBuffer(), s_Positions2);
}