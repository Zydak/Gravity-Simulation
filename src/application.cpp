#include "application.h"

#include "objects/planet.h"
#include "objects/star.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include <stbimage/stb_image.h>
#include <future>

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
    alignas(16) glm::vec4 lightColor{1.0f, 1.0f, 1.0f, 3.0f}; // w is light intesity
};

Application::Application()
{
    m_GlobalPool = DescriptorPool::Builder(m_Device)
        .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2 + 3) // * 2 for imgui
        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2 + 3)
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
        VkDescriptorBufferInfo bufferInfo = uboBuffers[i]->DescriptorInfo();
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
    int TargetLock = 1;

    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();

        auto now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float, std::chrono::seconds::period>(now - lastUpdate).count();
        lastUpdate = now;
        accumulator += delta;
        FPSaccumulator += delta;
        OrbitAccumulator += delta;

        if (auto commandBuffer = m_Renderer->BeginFrame({0.02f, 0.02f, 0.02f})) 
        {
            int frameIndex = m_Renderer->GetFrameIndex();
            
            FrameInfo frameInfo{};
            frameInfo.frameIndex = frameIndex;
            frameInfo.camera = &m_Camera;
            frameInfo.frameTime = delta;
            frameInfo.commandBuffer = commandBuffer;
            frameInfo.globalDescriptorSet = globalDescriptorSets[frameIndex];
            frameInfo.gameObjects = m_GameObjects;
            frameInfo.globalDescriptorSetLayout = globalSetLayout.get();
            frameInfo.globalDescriptorPool = m_GlobalPool.get();
            frameInfo.sampler = &m_Sampler;

            static int speed = 1;
            static int stepCount = 1;
            while (accumulator > 0.016f)
            {
                for(int i = 0; i < speed; i++)
                {
                    Update(frameInfo, 0.016f, stepCount);
                }
                accumulator -= 0.016f;
            }

            float aspectRatio = m_Renderer->GetAspectRatio();
            m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 50000.0f);
            m_CameraController.Update(0.016f, m_Camera, m_GameObjects[TargetLock]->GetObjectTransform().translation);
            m_Camera.SetViewTarget(m_GameObjects[TargetLock]->GetObjectTransform().translation);
            GlobalUbo ubo{};
            ubo.projection = m_Camera.GetProjection();
            ubo.view = m_Camera.GetView();
            uboBuffers[frameIndex]->WriteToBuffer(&ubo);
            uboBuffers[frameIndex]->Flush();

            // ------------------- RENDER PASS -----------------
            m_Renderer->BeginSwapChainRenderPass(commandBuffer, {0.02f, 0.02f, 0.02f});

            m_Renderer->RenderGameObjects(frameInfo);
            //m_Renderer->RenderSimpleGeometry(frameInfo, m_Obj.get());

            ImGui_ImplVulkan_NewFrame();
            ImGui_ImplGlfw_NewFrame();
            ImGui::NewFrame();
 
            ImGui::Begin("Settings", (bool*)false, 0);
            ImGui::SliderInt("Speed", &speed, 1, 200);
            ImGui::SliderInt("StepCount", &stepCount, 1, 2500);
            if (FPSaccumulator > 0.5f)
            {
                FPS = 1/frameInfo.frameTime;
                FPSaccumulator -= 0.5f;
            }
            ImGui::Text("FPS %.1f", FPS);
            for (auto& kv : m_GameObjects)
            {
                auto& obj = kv.second;
                ImGui::Text("Obj ID: %d Pos: x %f | y %f, z %f", 
                    obj->GetObjectID(), 
                    obj->GetObjectTransform().translation.x,
                    obj->GetObjectTransform().translation.y,
                    obj->GetObjectTransform().translation.z
                );
                ImGui::Text("velocity ID: %d velocity: x %f | y %f, z %f", 
                    obj->GetObjectID(), 
                    obj->GetObjectProperties().velocity.x,
                    obj->GetObjectProperties().velocity.y,
                    obj->GetObjectProperties().velocity.z
                );
                
                if (ImGui::Button(std::string("Camera Lock on " + std::to_string(obj->GetObjectID())).c_str()))
                {
                    TargetLock = obj->GetObjectID();
                }
                ImGui::DragFloat((std::to_string(obj->GetObjectID()) + std::string(" Obj Mass")).c_str(), &obj->GetObjectProperties().mass, 0.05f);
            }
            ImGui::Text("Camera Position: x %0.1f y %0.1f z %0.1f", 
                m_Camera.m_Transform.translation.x,
                m_Camera.m_Transform.translation.y,
                m_Camera.m_Transform.translation.z);

            ImGui::End();
 
            ImGui::Render();
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);

            m_Renderer->EndSwapChainRenderPass(commandBuffer);
            m_Renderer->EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void Application::LoadGameObjects()
{
    int ID = 0;
    ObjectInfo objInfo{};
    objInfo.descriptorPool = m_GlobalPool.get();
    objInfo.device = &m_Device;
    objInfo.sampler = &m_Sampler;

    Properties properties0{};
    properties0.velocity = {0.0f, 0.0f, 0.0f};
    properties0.mass = 5000;
    properties0.isStatic = true;
    properties0.orbitTraceLenght = 0;
    properties0.rotationSpeed = {0.0f, 0.01f, 0.0f};

    Transform transform0{};
    transform0.translation = {0.0f, 0.0f, 0.0f};
    transform0.scale = glm::vec3{1.0f, 1.0f, 1.0f}*5.0f;
    transform0.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj0 = std::make_unique<Star>(ID++, objInfo, "assets/models/smooth_sphere.obj", transform0, properties0, "assets/textures/white.png");
    m_GameObjects.emplace(obj0->GetObjectID(), std::move(obj0));

    Properties properties1{};
    properties1.velocity = {0.0f, 0.0f, -10.5f};
    properties1.mass = 1000;
    properties1.isStatic = false;
    properties1.orbitTraceLenght = 200;
    properties1.rotationSpeed = {0.0f, 0.05f, 0.0f};

    Transform transform1{};
    transform1.translation = {250.0f, 0.0f, 0.0f};
    transform1.scale = glm::vec3{1.0f, 1.0f, 1.0f} * properties1.mass/200.0f;
    transform1.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj1 = std::make_unique<Planet>(ID++, objInfo, "assets/models/smooth_sphere.obj", transform1, properties1, "assets/textures/blue.png");
    m_GameObjects.emplace(obj1->GetObjectID(), std::move(obj1));

    Properties properties2{};
    properties2.velocity = {0.0f, 0.0f, 13.5f};
    properties2.mass = 5;
    properties2.isStatic = false;
    properties2.orbitTraceLenght = 200;
    properties2.rotationSpeed = {0.0f, 0.05f, 0.0f};

    Transform transform2{};
    transform2.translation = {-100.0f, 0.0f, 0.0f};
    transform2.scale = glm::vec3{1.0f, 1.0f, 1.0f} * properties2.mass/6.0f;
    transform2.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj2 = std::make_unique<Planet>(ID++, objInfo, "assets/models/smooth_sphere.obj", transform2, properties2, "assets/textures/red.png");
    m_GameObjects.emplace(obj2->GetObjectID(), std::move(obj2));

    // Simple Geometry
    //
    //const std::vector<SimpleModel::Vertex> vertices = 
    //{
    //    {{-1.0f, -1.0f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    //    {{ 1.0f, -1.0f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    //    {{ 1.0f,  1.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    //    {{-1.0f,  1.0f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}
    //};
    //const std::vector<uint32_t> indices = 
    //{
    //    0, 1, 2, 2, 3, 0
    //};
    //m_Obj = std::make_unique<SimpleModel>(m_Device, vertices, indices);
}

static void UpdateObj(Object* obj, std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta, uint32_t substeps)
{
    obj->Update(gameObjects, delta, substeps);
}

void Application::Update(FrameInfo frameInfo, float delta, uint32_t substeps)
{
    //std::vector<std::future<void>> futures;
    for (auto& kv: m_GameObjects)
    {
        auto& obj = kv.second;

        const float stepDelta = delta / substeps;
        std::async(std::launch::async, UpdateObj, obj.get(), m_GameObjects, stepDelta, substeps);
        //futures.push_back(std::async(std::launch::async, UpdateObj, obj.get(), m_GameObjects, stepDelta, substeps));
        //obj->Update(m_GameObjects, stepDelta, substeps);
        auto sralnia = obj->GetObjectTransform();
        obj->OrbitUpdate(frameInfo.commandBuffer);
        obj->GetObjectTransform().rotation += obj->GetObjectProperties().rotationSpeed;
    }
}