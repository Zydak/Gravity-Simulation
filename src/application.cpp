#include "application.h"

#include "objects/sphere.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "stbimage/stb_image.h"

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include <stbimage/stb_image.h>
#include <future>
#include <array>
#include <chrono>

static float orbitAccumulator = 0;

const char* Skyboxes[] = { "Milky Way", "Nebula", "Stars", "Red Galaxy"};
static int skyboxImageSelected = SkyboxTextureImage::Stars;

class Timer
{
public:
    std::chrono::time_point<std::chrono::high_resolution_clock> m_Start, m_End;

    Timer()
    {
        m_Start = std::chrono::high_resolution_clock::now();
    }

    ~Timer()
    {
        m_End = std::chrono::high_resolution_clock::now();
        double duration = (m_End - m_Start).count();
        std::cout << "Timer Took " << duration << "ns" << std::endl;;
    }
};

static void CheckVkResult(VkResult err)
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
    alignas(16) glm::vec4 lightColor{1.0f, 1.0f, 1.0f, 3.0f}; // w is light intensity
};

Application::Application()
{
    m_GlobalPool = DescriptorPool::Builder(m_Device)
        .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 3 + 3) // * 2 for ImGui
        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 3)
        .Build();
    
    LoadGameObjects();
    m_Skybox = std::make_unique<Skybox>(m_Device, skyboxImageSelected);
}

Application::~Application()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
void Application::Run()
{
	// Main Uniform Buffer Creation
    m_UboBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (auto & uboBuffer : m_UboBuffers)
    {
        uboBuffer = std::make_unique<Buffer>(
            m_Device,
            sizeof(GlobalUbo),
            1,
            VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
        );
        uboBuffer->Map();
    }

    auto globalSetLayout = DescriptorSetLayout::Builder(m_Device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

    m_SkyboxSetLayout = DescriptorSetLayout::Builder(m_Device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

	//Renderer Creation
    m_Renderer = std::make_unique<Renderer>(m_Window, m_Device, globalSetLayout->GetDescriptorSetLayout());

	// Main Descriptor Set Creation
    std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++)
    {
        VkDescriptorBufferInfo bufferInfo = m_UboBuffers[i]->DescriptorInfo();
        DescriptorWriter(*globalSetLayout, *m_GlobalPool)
            .WriteBuffer(0, &bufferInfo)
            .Build(globalDescriptorSets[i]);
    }

    VkDescriptorImageInfo skyboxDescriptor{};
    skyboxDescriptor.sampler = m_Skybox->GetCubemap().GetCubeMapImageSampler();
    skyboxDescriptor.imageView = m_Skybox->GetCubemap().GetCubeMapImageView();
    skyboxDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    DescriptorWriter(*m_SkyboxSetLayout, *m_GlobalPool)
        .WriteImage(0, &skyboxDescriptor)
        .Build(m_SkyboxDescriptorSet);

	// ImGui Creation
	ImGui::CreateContext();
	auto io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

	ImGui_ImplGlfw_InitForVulkan(m_Window.GetGLFWwindow(), true);
	ImGui_ImplVulkan_InitInfo info{};
	info.Instance = m_Device.GetInstance();
	info.PhysicalDevice = m_Device.GetPhysicalDevice();
	info.Device = m_Device.GetDevice();
	info.Queue = m_Device.GetGraphicsQueue();
	info.DescriptorPool = m_GlobalPool->GetDescriptorPool();
	info.Subpass = 0;
	info.MinImageCount = 2;
	info.ImageCount = m_Renderer->GetSwapChainImageCount();
	info.CheckVkResultFn = CheckVkResult;
	ImGui_ImplVulkan_Init(&info, m_Renderer->GetSwapChainRenderPass());

	VkCommandBuffer cmdBuffer;
    m_Device.BeginSingleTimeCommands(cmdBuffer);
	ImGui_ImplVulkan_CreateFontsTexture(cmdBuffer);
	m_Device.EndSingleTimeCommands(cmdBuffer);

	vkDeviceWaitIdle(m_Device.GetDevice());
	ImGui_ImplVulkan_DestroyFontUploadObjects();
    
    auto lastUpdate = std::chrono::high_resolution_clock::now();
	// Main Loop
    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();

        static int currentSkyboxImageSelected = skyboxImageSelected;
        if (currentSkyboxImageSelected != skyboxImageSelected)
        {
            currentSkyboxImageSelected = skyboxImageSelected;
            m_Skybox.reset(new Skybox(m_Device, skyboxImageSelected));
            VkDescriptorImageInfo skyboxDescriptor{};
            skyboxDescriptor.sampler = m_Skybox->GetCubemap().GetCubeMapImageSampler();
            skyboxDescriptor.imageView = m_Skybox->GetCubemap().GetCubeMapImageView();
            skyboxDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
            DescriptorWriter(*m_SkyboxSetLayout, *m_GlobalPool)
                .WriteImage(0, &skyboxDescriptor)
                .Overwrite(m_SkyboxDescriptorSet);
            lastUpdate = std::chrono::high_resolution_clock::now();
        }

		// Delta Time
        auto now = std::chrono::high_resolution_clock::now();
        float delta = std::chrono::duration<float, std::chrono::seconds::period>(now - lastUpdate).count();
        if (m_Pause)
        {
            lastUpdate = std::chrono::high_resolution_clock::now();
        }
        if (!m_Pause)
        {
            lastUpdate = now;
            m_MainLoopAccumulator += delta;
            orbitAccumulator += delta;
        }
        m_FPSaccumulator += delta;

		// Frame Begin
        if (auto commandBuffer = m_Renderer->BeginFrame({0.02f, 0.02f, 0.02f})) 
        {
            // Fill FrameInfo struct
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

			// Update Every 160ms(every frame with 60fps) independent of actual framerate
            while (m_MainLoopAccumulator > 0.016f && !m_Pause)
            {
                Update(frameInfo, 0.016f);
				m_MainLoopAccumulator -= 0.016f;
            }

			// Camera Update
            float aspectRatio = m_Renderer->GetAspectRatio();
            m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 1.0f, 50000000);
            m_CameraController.Update(0.016f, m_Camera, m_TargetLock, m_GameObjects);
            m_Camera.SetViewTarget(m_GameObjects[m_TargetLock]->GetObjectTransform().translation);
            
            // UBO update
            {
                GlobalUbo ubo{};
                ubo.projection = m_Camera.GetProjection();
                ubo.view = m_Camera.GetView();
                for (auto& kv : m_GameObjects)
                {
                    if (kv.second->GetObjectType() == OBJ_TYPE_STAR)
                    {
                        ubo.lightPosition = kv.second->GetObjectTransform().translation;
                    }
                }
                m_UboBuffers[frameIndex]->WriteToBuffer(&ubo);
                m_UboBuffers[frameIndex]->Flush();
            }

            // ------------------- RENDER PASS -----------------
            m_Renderer->BeginSwapChainRenderPass(commandBuffer, {0.0f, 0.0f, 0.0f});
            m_Renderer->RenderSkybox(frameInfo, *m_Skybox, m_SkyboxDescriptorSet); // Skybox has to be rendered first

            m_Renderer->RenderGameObjects(frameInfo);

            RenderImGui(frameInfo);

            m_Renderer->EndSwapChainRenderPass(commandBuffer);
            m_Renderer->EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

/**
 * @brief Loads specified game objects with their parameters
*/
void Application::LoadGameObjects()
{
    // Main texture sampler creation
    m_Sampler.CreateSimpleSampler();

    int id = 0;
    ObjectInfo objInfo{};
    objInfo.descriptorPool = m_GlobalPool.get();
    objInfo.device = &m_Device;
    objInfo.sampler = &m_Sampler;

    //
    // EARTH
    //
    {
        Properties properties{};
        properties.velocity = {0.0f, 0.0f, 0.0f};
        properties.mass = 5.972 * pow(10, 24);
        properties.isStatic = false;
        properties.orbitTraceLenght = 200;
        properties.rotationSpeed = {0.0f, 0.05f, 0.0f};
        properties.radius = 6371.0f;
        
        properties.objType = OBJ_TYPE_PLANET;

        Transform transform{};
        transform.translation = {0.0f, 0.0f, 0.0f};
        transform.rotation = {0.0f, 0.0f, 0.0f};
        std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo, 
            "assets/models/smooth_sphere.obj", transform, properties, "assets/textures/red.png");
        m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
    }

    //
    // MOON
    //
    {
        Properties properties{};
        properties.velocity = {0.0f, 0.0f, 8.0}; // km/s
        properties.mass = 7.348 * pow(10, 22);
        properties.isStatic = false;
        properties.orbitTraceLenght = 200;
        properties.rotationSpeed = {0.0f, 0.05f, 0.0f};
        properties.objType = OBJ_TYPE_PLANET;
        properties.radius = 1737.5f;

        Transform transform{};
        transform.translation = {384400.0f, 0.0f, 0.0f};
        transform.rotation = {0.0f, 0.0f, 0.0f};
        std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo, 
            "assets/models/smooth_sphere.obj", transform, properties, "assets/textures/blue.png");
        m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
    }
}

/**
 * @brief Updates game objects and their orbit traces
 */
void Application::Update(const FrameInfo& frameInfo, float delta)
{
    static int OrbitUpdateCount = 0;
    double substepDelta = delta / m_StepCount;
    for (int i = 0; i < m_GameSpeed; i++)
    {
        for (int j = 0; j < m_StepCount; j++)
        {
            // Timer timer;
            for (auto& kv: m_GameObjects)
            {
                auto& objA = kv.second;
                
                if (m_GameObjects.size() == 1) // if there is only one object still apply it's velocity
                {
                    objA->GetObjectTransform().translation += objA->GetObjectProperties().velocity;
                }
                else
                {
                    for (auto& kv1: m_GameObjects)
                    {
                        auto& objB = kv1.second;
                        if (objA == objB)
                            continue;
                        
                        // convert translations from km to m
                        glm::dvec3 offset = objA->GetObjectTransform().translation*1000.0 - objB->GetObjectTransform().translation*1000.0;
                        double distanceSquared = glm::dot(offset, offset);
                        std::cout << sqrt(distanceSquared) << std::endl;
                        if (std::sqrt(distanceSquared) < objA->GetObjectProperties().radius + objB->GetObjectProperties().radius)
                        {
                            std::cout << "HIT" << std::endl;
                        }

                        double G = 6.67 / (pow(10, 11));
                        double force = G * objA->GetObjectProperties().mass * objB->GetObjectProperties().mass / distanceSquared;
                        glm::dvec3 trueForce = force * offset / glm::sqrt(distanceSquared);
                        objA->GetObjectProperties().velocity += -trueForce / (objA->GetObjectProperties().mass * substepDelta)/1000.0;// convert back to km
                        objA->GetObjectTransform().translation += objA->GetObjectProperties().velocity;

                        objB->GetObjectProperties().velocity += trueForce / (objB->GetObjectProperties().mass * substepDelta)/1000.0;// convert back to km
                        objB->GetObjectTransform().translation += objB->GetObjectProperties().velocity;
                    }
                }
                
                objA->GetObjectTransform().rotation += objA->GetObjectProperties().rotationSpeed;
            }
        }
        
        // Update orbits less frequently(after 2 obj updates in this case) to make them longer
        OrbitUpdateCount = (OrbitUpdateCount + 1) % 2;
        if (OrbitUpdateCount == 0)
        {
            for (auto& kv : m_GameObjects)
            {
                auto& obj = kv.second;
                obj->OrbitUpdate(frameInfo.commandBuffer);
            }
        }
    }
}

void Application::RenderImGui(const FrameInfo& frameInfo)
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::Begin("Settings", (bool*)false, 0);

    if (m_FPSaccumulator > 0.5f)
    {
        m_FPS = 1/frameInfo.frameTime;
        m_FPSaccumulator -= 0.5f;
    }
    ImGui::Text("FPS %.1f (%fms)", m_FPS, frameInfo.frameTime);
    ImGui::Checkbox("Pause", &m_Pause);

    ImGui::Combo("Skybox", &skyboxImageSelected, Skyboxes, IM_ARRAYSIZE(Skyboxes));

    ImGui::SliderInt("Speed", &m_GameSpeed, 1, 500);
    ImGui::SliderInt("StepCount", &m_StepCount, 1, 2500);
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
            m_TargetLock = obj->GetObjectID();
        }
    }
    ImGui::Text("Camera Position: x %0.1f y %0.1f z %0.1f", 
        m_Camera.m_Transform.translation.x,
        m_Camera.m_Transform.translation.y,
        m_Camera.m_Transform.translation.z);

    ImGui::End();
 
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}