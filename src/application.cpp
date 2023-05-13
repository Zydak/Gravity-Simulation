#include "application.h"

#include "objects/sphere.h"
#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>

#include "stbimage/stb_image.h"

#include "imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_vulkan.h"
#include <future>
#include <array>
#include <chrono>
#include "defines.h"

static float orbitAccumulator = 0;

static const char* Skyboxes[] = { "Milky Way", "Nebula", "Stars", "Red Galaxy"};// I tried to make this map but imgui only works with c-string array
static int skyboxImageSelected = SkyboxTextureImage::Stars;

static double realTime = 0;

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
        double duration = (m_End - m_Start).count() / 1000.0;
        std::cout << "Timer Took " << duration << "ms" << std::endl;
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
        .SetMaxSets((SwapChain::MAX_FRAMES_IN_FLIGHT+8) * 2)// * 2 for ImGui
        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, (SwapChain::MAX_FRAMES_IN_FLIGHT + 8) * 2)
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
        .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
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
        VkDescriptorImageInfo iconDescriptor{};
        iconDescriptor.sampler = m_Sampler.GetSampler();
        iconDescriptor.imageView = m_IconImage.GetImageView();
        iconDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

        VkDescriptorBufferInfo bufferInfo = m_UboBuffers[i]->DescriptorInfo();
        DescriptorWriter(*globalSetLayout, *m_GlobalPool)
            .WriteBuffer(0, &bufferInfo)
            .WriteImage(1, &iconDescriptor)
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
        #ifndef FAST_LOAD
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
        #endif

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
        if (auto commandBuffer = m_Renderer->BeginFrame())
        {
            // Fill FrameInfo struct
            FrameInfo frameInfo{};
            int frameIndex = m_Renderer->GetFrameIndex();
            frameInfo.camera = &m_Camera;
            frameInfo.frameTime = delta;
            frameInfo.commandBuffer = commandBuffer;
            frameInfo.globalDescriptorSet = globalDescriptorSets[frameIndex];
            frameInfo.gameObjects = m_GameObjects;

			// Update Every 160ms(every frame with 60fps) independent of actual framerate
            while (m_MainLoopAccumulator > 0.016f && !m_Pause)
            {
                Update(frameInfo, 60.0); // making delta value larger will speed up simulation while loosing its accuracy but I guess making it 0.016 and waiting 10 thousand days for some orbit to complete is not good idea so we have to do that
				m_MainLoopAccumulator -= 0.016f;
            }
            frameInfo.offset = m_GameObjects[m_TargetLock]->GetObjectTransform().translation;
            
			// Camera Update
            float aspectRatio = m_Renderer->GetAspectRatio();
            m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.000001f, 1000.0f);
            m_CameraController.Update(0.016f, m_Camera, m_TargetLock, m_GameObjects);
            m_Camera.SetViewTarget({0.0, 0.0, 0.0});
            
            // UBO update
            {
                GlobalUbo ubo{};
                ubo.projection = m_Camera.GetProjection();
                ubo.view = m_Camera.GetView();
                
                for (auto& kv : m_GameObjects)
                {
                    if (kv.second->GetObjectType() == OBJ_TYPE_STAR)
                    {
                        ubo.lightPosition = glm::vec4(kv.second->GetObjectTransform().translation/SCALE_DOWN - glm::dvec3(frameInfo.offset/SCALE_DOWN), 1.0);
                    }
                }
                m_UboBuffers[frameIndex]->WriteToBuffer(&ubo);
                m_UboBuffers[frameIndex]->Flush();
            }
            

            // ------------------- RENDER PASS -----------------
            m_Renderer->BeginSwapChainRenderPass(commandBuffer, {0.0f, 0.0f, 0.0f});
            #ifndef FAST_LOAD
                m_Renderer->RenderSkybox(frameInfo, *m_Skybox, m_SkyboxDescriptorSet); // Skybox has to be rendered first
            #endif

            m_Renderer->RenderGameObjects(frameInfo);

            RenderImGui(frameInfo);

            m_Renderer->EndSwapChainRenderPass(commandBuffer);
            m_Renderer->EndFrame();
        }
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

#pragma region Planets

/**
 * @brief Loads game objects with their parameters
 * @note All the data comes from https://nssdc.gsfc.nasa.gov/planetary/factsheet/
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
	// SUN
	//
	{
		Properties properties{};
        properties.label = "Sun";
        properties.orbitUpdateFrequency = 0; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0f, 0.0f, 0.0}; // km/s
		properties.mass = 1.99 * pow(10, 30); // kg
		properties.orbitTraceLenght = 0;
		properties.rotationSpeed = {0.0f, 0.0f, 0.0f}; // Degree per hour
		properties.objType = OBJ_TYPE_STAR;
		properties.radius = 695508.0f; // km
		properties.color = {0.98f, 0.97f, 0.1f}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {0.0f, 0.0f, 0.0f}; // km
		transform.rotation = {0.0f, 0.0f, 0.0f}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties);
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

	//
	// MERCURY
	//
	{
		Properties properties{};
        properties.label = "Mercury";
        properties.orbitUpdateFrequency = 100; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 58.97}; // km/s
		properties.mass = 0.33010 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 2440.0f; // km
		properties.color = {0.788, 0.627, 0.42}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {46000000.0, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties, "assets/textures/mercury.jpg");
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

	//
	// VENUS
	//
	{
		Properties properties{};
        properties.label = "Venus";
        properties.orbitUpdateFrequency = 100; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 35.26}; // km/s
		properties.mass = 4.8673 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 6051.8; // km
		properties.color = {0.941, 0.78, 0.263}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {107480000.0, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties, "assets/textures/venus.jpg");
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

    //
    // EARTH
    //
    {
        Properties properties{};
        properties.label = "Earth";
        properties.orbitUpdateFrequency = 200; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
        properties.velocity = {0.0, 0.0, 30.29}; // km/s
        properties.mass = 5.9722 * pow(10, 24); // kg
        properties.orbitTraceLenght = 1000;
        properties.rotationSpeed = {0.0, 15.0, 0.0}; // Degree per hour
        properties.objType = OBJ_TYPE_PLANET;
        properties.radius = 6378.137; // km
        properties.color = {0.5, 0.8, 0.94}; // color of the icon and orbit trace not planet itself

        Transform transform{};
        transform.translation = {147095000.0, 0.0, 0.0}; // km
        transform.rotation = {0.0, 0.0, 180.0}; // starting rotation in degrees
        std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo, 
            "assets/models/sphere.obj", transform, properties, "assets/textures/earth.jpg");
        m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
    }

    //
    // MOON
    //
    {
        Properties properties{};
        properties.label = "Moon";
        properties.orbitUpdateFrequency = 200; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
        properties.velocity = {0.0, 0.0, 30.29 + 1.082}; // km/s
        properties.mass = 0.07346 * pow(10, 24); // kg
        properties.orbitTraceLenght = 1000;
        properties.rotationSpeed = {0.0f, 0.0, 0.0f}; // Degree per hour
        properties.objType = OBJ_TYPE_PLANET;
        properties.radius = 1737.5f; // km
        properties.color = {0.678, 0.678, 0.678}; // color of the icon and orbit trace not planet itself

        Transform transform{};
        properties.orbitUpdateFrequency = 100;
        transform.translation = {363300  + 147095000.0, 0.0f, 0.0f}; // km
        transform.rotation = {0.0f, 180.0f, 0.0f}; // starting rotation in degrees
        std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo, 
            "assets/models/sphere.obj", transform, properties, "assets/textures/moon.jpg");
        m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
    }

	//
	// MARS
	//
	{
		Properties properties{};
        properties.label = "Mars";
        properties.orbitUpdateFrequency = 300; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 26.50}; // km/s
		properties.mass = 0.64169 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 3396.2; // km
		properties.color = {0.988, 0.537, 0.333}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {206650000.0, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties, "assets/textures/mars.jpg");
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

	//
	// JUPITER
	//
	{
		Properties properties{};
        properties.label = "Jupiter";
        properties.orbitUpdateFrequency = 5000; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 13.72}; // km/s
		properties.mass = 1898.13 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 69911.0; // km
		properties.color = {0.839, 0.718, 0.541}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {740595000.0, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties);
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

	//
	// SATURN
	//
	{
		Properties properties{};
        properties.label = "Saturn";
        properties.orbitUpdateFrequency = 10000; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 10.14}; // km/s
		properties.mass = 568.32 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 60268.0; // km
		properties.color = {0.961, 0.906, 0.827}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {1357554000, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties, "assets/textures/saturn.png");
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

	//
	// UR ANUS
	//
	{
		Properties properties{};
        properties.label = "Uranus";
        properties.orbitUpdateFrequency = 30000; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 7.13}; // km/s
		properties.mass = 86.811 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 25362.0; // km
		properties.color = {0.659, 0.835, 0.859}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {2732696000, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties, "assets/textures/uranus.jpg");
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

    //
	// NEPTUNE
	//
	{
		Properties properties{};
        properties.label = "Neptune";
        properties.orbitUpdateFrequency = 60000; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 5.47}; // km/s
		properties.mass = 102.409 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 24622.0; // km
		properties.color = {0.4, 0.49, 0.89}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {4471050000, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties, "assets/textures/neptune.jpg");
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

    //
	// PLUTO
	//
	{
		Properties properties{};
        properties.label = "Pluto";
        properties.orbitUpdateFrequency = 100000; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 3.71}; // km/s
		properties.mass = 0.01303 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 1188.0; // km
		properties.color = {0.839, 0.514, 0.514}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {7304326000, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties);
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}

    //
	// ERIS
	//
	{
		Properties properties{};
        properties.label = "Eris";
        properties.orbitUpdateFrequency = 300000; // Update orbits less frequently to make them longer. It is usefull if planets are far from sun and move really slow
		properties.velocity = {0.0, 0.0, 3.435}; // km/s
		properties.mass = 0.016 * pow(10, 24); // kg
		properties.orbitTraceLenght = 1000;
		properties.rotationSpeed = {0.0, 0.0, 0.0}; // Degree per hour
		properties.objType = OBJ_TYPE_PLANET;
		properties.radius = 1163.0; // km
		properties.color = {0.776, 0.788, 0.839}; // color of the icon and orbit trace not planet itself

		Transform transform{};
		transform.translation = {14594770000, 0.0, 0.0}; // km
		transform.rotation = {0.0, 180.0, 0.0}; // starting rotation in degrees
		std::unique_ptr<Object> obj = std::make_unique<Sphere>(id++, objInfo,
			"assets/models/sphere.obj", transform, properties);
		m_GameObjects.emplace(obj->GetObjectID(), std::move(obj));
	}
}

#pragma endregion Planets

/**
 * @brief Updates game objects and their orbit traces
 */
void Application::Update(const FrameInfo& frameInfo, float delta)
{
    //m_Pause = true;
    static uint32_t orbitUpdateCount = 0;

    double substepDelta = delta / (double)m_StepCount;
    for (int i = 0; i < m_GameSpeed; i++)
    {
        for (int j = 0; j < m_StepCount; j++)
        {
            // Loop through pairs of objects and applies velocity to them
            for (auto iterA = m_GameObjects.begin(); iterA != m_GameObjects.end(); iterA++)
            {
                auto& objA = iterA->second;
                
                if (m_GameObjects.size() == 1) // if there is only one object still apply it's velocity
                {
                    objA->GetObjectTransform().translation += objA->GetObjectProperties().velocity;
                }
                else
                {
                    for (auto iterB = iterA; iterB != m_GameObjects.end(); iterB++)
                    {
                        auto& objB = iterB->second;
                        if (objA == objB)
                            continue;
                        
                        // convert translations from km to m
                        glm::dvec3 offset = objA->GetObjectTransform().translation*1000.0 - objB->GetObjectTransform().translation*1000.0;
                        double distanceSquared = glm::dot(offset, offset);
                        if (std::sqrt(distanceSquared)/1000.0 < objA->GetObjectProperties().radius + objB->GetObjectProperties().radius)
                        {
                            std::cout << "HIT" << std::endl;
                            // something should go in here
                        }
                        double G = 6.67 / (pow(10, 11));
                        double force = G * objA->GetObjectProperties().mass * objB->GetObjectProperties().mass / distanceSquared;
                        glm::dvec3 trueForce = force * offset / glm::sqrt(distanceSquared);
                        objA->GetObjectProperties().velocity += substepDelta * -trueForce / objA->GetObjectProperties().mass / 1000.0;// convert back to km
                        objB->GetObjectProperties().velocity += substepDelta * trueForce / objB->GetObjectProperties().mass / 1000.0;// convert back to km
                    }
                }
                
                objA->GetObjectTransform().rotation += objA->GetObjectProperties().rotationSpeed * (double)substepDelta;
            }

            // Update each object position by it's final velocity
            for (auto& kv : m_GameObjects)
            {
                auto& obj = kv.second;
                obj->GetObjectTransform().translation += substepDelta * obj->GetObjectProperties().velocity;
            }
        }
        // Update orbits less frequently(after 5000 obj updates in this case) to make them longer
        orbitUpdateCount = (orbitUpdateCount + 1) % uint32_t(0-1);
        
        for (auto& kv : m_GameObjects)
        {
            auto& obj = kv.second;
            auto x = orbitUpdateCount;
            auto d = obj->GetObjectOrbitUpdateFreq();
            if (orbitUpdateCount % obj->GetObjectOrbitUpdateFreq() == 0)
            {
                obj->OrbitUpdate(frameInfo.commandBuffer);
            }
        }

        realTime += ((double)delta/3600);
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
    ImGui::Text("Simulation Time: %.2f hours | %.0f days | %.0f years", std::floor(realTime), std::floor(realTime/24), std::floor(realTime/24/365.25));

    ImGui::Combo("Skybox", &skyboxImageSelected, Skyboxes, IM_ARRAYSIZE(Skyboxes));

    ImGui::SliderInt("Speed", &m_GameSpeed, 1, 10000);
    ImGui::SliderInt("StepCount", &m_StepCount, 1, 20);
    for (auto& kv : m_GameObjects)
    {
        auto& obj = kv.second;
        double velocity = sqrt(pow(obj->GetObjectProperties().velocity.z, 2)) + sqrt(pow(obj->GetObjectProperties().velocity.y, 2)) + sqrt(pow(obj->GetObjectProperties().velocity.x, 2));
        ImGui::Text("%s velocity: %f km/s", 
            obj->GetObjectLabel().c_str(),
            velocity
        );

        std::string text = "Camera Lock on " + obj->GetObjectLabel();          
        if (ImGui::Button(text.c_str()))
        {
            m_TargetLock = obj->GetObjectID();
        }
    }
    
    ImGui::End();
 
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}
