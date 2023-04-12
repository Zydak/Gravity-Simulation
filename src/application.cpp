#include "application.h"

#include "objects/planet.h"
#include "objects/star.h"

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

VkSampler cubeMapSampler;
VkImage cubeMapImage;
VkDeviceMemory cubeMapImageMemory;
VkImageView cubeMapImageView;

void Application::CreateCubemap()
{
    std::array<std::string, 6> filepaths{};
    filepaths[0] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_NegativeZ.png";
    filepaths[1] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_PositiveZ.png";
    filepaths[2] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_PositiveY.png";
    filepaths[3] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_NegativeY.png";
    filepaths[4] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_PositiveX.png";
    filepaths[5] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_NegativeX.png";

    // filepaths[0] = "/home/zydak/Desktop/front.jpg";
    // filepaths[1] = "/home/zydak/Desktop/back.jpg";
    // filepaths[2] = "/home/zydak/Desktop/top.jpg";
    // filepaths[3] = "/home/zydak/Desktop/bottom.jpg";
    // filepaths[4] = "/home/zydak/Desktop/left.jpg";
    // filepaths[5] = "/home/zydak/Desktop/right.jpg";

    std::array<stbi_uc*, 6> pixels;

    int texWidth, texHeight, texChannels;
    for (int i = 0; i < 6; i++)
    {
        pixels[i] = stbi_load(filepaths[i].c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
    }

    VkDeviceSize allSize = texWidth * texHeight * 4 * 6;
    VkDeviceSize imageSize = texWidth * texHeight * 4;

    VkMemoryAllocateInfo memAllocInfo{};
    memAllocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	VkMemoryRequirements memReqs;

	// Create a host-visible staging buffer that contains the raw image data
	VkBuffer stagingBuffer;
	VkDeviceMemory stagingMemory;

    VkBufferCreateInfo bufferCreateInfo{};
    bufferCreateInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferCreateInfo.size = allSize;
	// This buffer is used as a transfer source for the buffer copy
	bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	bufferCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

    vkCreateBuffer(m_Device.GetDevice(), &bufferCreateInfo, nullptr, &stagingBuffer);

    vkGetBufferMemoryRequirements(m_Device.GetDevice(), stagingBuffer, &memReqs);
	memAllocInfo.allocationSize = memReqs.size;
	// Get memory type index for a host visible buffer
	memAllocInfo.memoryTypeIndex = m_Device.FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	vkAllocateMemory(m_Device.GetDevice(), &memAllocInfo, nullptr, &stagingMemory);
	vkBindBufferMemory(m_Device.GetDevice(), stagingBuffer, stagingMemory, 0);

    // Copy texture data into staging buffer
	uint8_t *data;
	vkMapMemory(m_Device.GetDevice(), stagingMemory, 0, memReqs.size, 0, (void **)&data);
    for (int i = 0; i < 6; i++)
    {
	    memcpy((char*)data + (imageSize*i), pixels[i], static_cast<size_t>(imageSize));
    }
	vkUnmapMemory(m_Device.GetDevice(), stagingMemory);

    // Create optimal tiled target image
	VkImageCreateInfo imageCreateInfo{};
    imageCreateInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageCreateInfo.imageType = VK_IMAGE_TYPE_2D;
	imageCreateInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
	imageCreateInfo.mipLevels = 1;
	imageCreateInfo.samples = VK_SAMPLE_COUNT_1_BIT;
	imageCreateInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
	imageCreateInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
	imageCreateInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
	imageCreateInfo.extent = { texWidth, texHeight, 1 };
	imageCreateInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	// Cube faces count as array layers in Vulkan
	imageCreateInfo.arrayLayers = 6;
	// This flag is required for cube map images
	imageCreateInfo.flags = VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT;

    if (vkCreateImage(m_Device.GetDevice(), &imageCreateInfo, nullptr, &cubeMapImage) != VK_SUCCESS)
    {
        throw std::runtime_error("Sralnia");
    }

    vkGetImageMemoryRequirements(m_Device.GetDevice(), cubeMapImage, &memReqs);
    memAllocInfo.allocationSize = memReqs.size;
	memAllocInfo.memoryTypeIndex = m_Device.FindMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    vkAllocateMemory(m_Device.GetDevice(), &memAllocInfo, nullptr, &cubeMapImageMemory);
	vkBindImageMemory(m_Device.GetDevice(), cubeMapImage, cubeMapImageMemory, 0);

    //  for (uint32_t face = 0; face < 6; face++)
	// 	{
	// 		// Calculate offset into staging buffer for the current mip level and face
	// 		size_t offset = 4 * 128 * 128;
	// 		VkBufferImageCopy bufferCopyRegion = {};
	// 		bufferCopyRegion.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
	// 		bufferCopyRegion.imageSubresource.mipLevel = 0;
	// 		bufferCopyRegion.imageSubresource.baseArrayLayer = face;
	// 		bufferCopyRegion.imageSubresource.layerCount = 1;
	// 		bufferCopyRegion.imageExtent.width = texWidth;
	// 		bufferCopyRegion.imageExtent.height = texHeight;
	// 		bufferCopyRegion.imageExtent.depth = 1;
	// 		bufferCopyRegion.bufferOffset = offset;
	// 		bufferCopyRegions.push_back(bufferCopyRegion);
	// 	}

    VkImageSubresourceRange subresourceRange = {};
    subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    subresourceRange.baseMipLevel = 0;
    subresourceRange.levelCount = 1;
    subresourceRange.baseArrayLayer = 0;
    subresourceRange.layerCount = 6;

    Image::TransitionImageLayout(m_Device, cubeMapImage, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, subresourceRange);

    VkCommandBuffer commandBuffer;
    m_Device.BeginSingleTimeCommands(commandBuffer);

    VkBufferImageCopy region{};
    region.bufferOffset = 0;
    region.bufferRowLength = 0;
    region.bufferImageHeight = 0;
    region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    region.imageSubresource.mipLevel = 0;
    region.imageSubresource.baseArrayLayer = 0;
    region.imageSubresource.layerCount = 6;
    region.imageOffset = {0, 0, 0};
    region.imageExtent = {
        texWidth,
        texHeight,
        1
    };

    vkCmdCopyBufferToImage(
        commandBuffer,
        stagingBuffer,
        cubeMapImage,
        VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        1,
        &region
    );

    m_Device.EndSingleTimeCommands(commandBuffer);

    Image::TransitionImageLayout(m_Device, cubeMapImage, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, subresourceRange);

    // Create sampler
	VkSamplerCreateInfo sampler{};
    sampler.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	sampler.magFilter = VK_FILTER_LINEAR;
	sampler.minFilter = VK_FILTER_LINEAR;
	sampler.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
	sampler.addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
	sampler.addressModeV = sampler.addressModeU;
	sampler.addressModeW = sampler.addressModeU;
	sampler.mipLodBias = 0.0f;
	sampler.compareOp = VK_COMPARE_OP_NEVER;
	sampler.minLod = 0.0f;
	sampler.maxLod = 0.0f;
	sampler.borderColor = VK_BORDER_COLOR_FLOAT_OPAQUE_WHITE;
	sampler.maxAnisotropy = m_Device.GetDeviceProperties().limits.maxSamplerAnisotropy;
    sampler.anisotropyEnable = VK_TRUE;
    vkCreateSampler(m_Device.GetDevice(), &sampler, nullptr, &cubeMapSampler);

    
    // Create image view
	VkImageViewCreateInfo view{};
    view.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	// Cube map view type
	view.viewType = VK_IMAGE_VIEW_TYPE_CUBE;
	view.format = VK_FORMAT_R8G8B8A8_SRGB;
	view.subresourceRange = { VK_IMAGE_ASPECT_COLOR_BIT, 0, 1, 0, 1 };
	// 6 array layers (faces)
	view.subresourceRange.layerCount = 6;
	// Set number of mip levels
	view.subresourceRange.levelCount = 1;
	view.image = cubeMapImage;
	vkCreateImageView(m_Device.GetDevice(), &view, nullptr, &cubeMapImageView);

    vkFreeMemory(m_Device.GetDevice(), stagingMemory, nullptr);
    vkDestroyBuffer(m_Device.GetDevice(), stagingBuffer, nullptr);

    for (int i = 0; i < 6; i++)
    {
        stbi_image_free(pixels[i]);
    }
}

static float orbitAccumulator = 0;

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
        .SetMaxSets(SwapChain::MAX_FRAMES_IN_FLIGHT * 2 + 3) // * 2 for ImGui
        .SetPoolFlags(VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, SwapChain::MAX_FRAMES_IN_FLIGHT)
        .AddPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, SwapChain::MAX_FRAMES_IN_FLIGHT * 2 + 3)
        .Build();
    
    LoadGameObjects();
    CreateCubemap();
}

Application::~Application()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();

    vkDestroySampler(m_Device.GetDevice(), cubeMapSampler, nullptr);
    vkDestroyImage(m_Device.GetDevice(), cubeMapImage, nullptr);
    vkFreeMemory(m_Device.GetDevice(), cubeMapImageMemory, nullptr);
    vkDestroyImageView(m_Device.GetDevice(), cubeMapImageView, nullptr);
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
        .AddBinding(1, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

	//Renderer Creation
    m_Renderer = std::make_unique<Renderer>(m_Window, m_Device, globalSetLayout->GetDescriptorSetLayout());

	// Main Descriptor Set Creation
    std::vector<VkDescriptorSet> globalDescriptorSets(SwapChain::MAX_FRAMES_IN_FLIGHT);
    for (int i = 0; i < globalDescriptorSets.size(); i++)
    {
        VkDescriptorImageInfo skyboxDescriptor{};
        skyboxDescriptor.sampler = cubeMapSampler;
        skyboxDescriptor.imageView = cubeMapImageView;
        skyboxDescriptor.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        VkDescriptorBufferInfo bufferInfo = m_UboBuffers[i]->DescriptorInfo();
        DescriptorWriter(*globalSetLayout, *m_GlobalPool)
            .WriteBuffer(0, &bufferInfo)
            .WriteImage(1, &skyboxDescriptor)
            .Build(globalDescriptorSets[i]);
    }

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
                Update(frameInfo, 0.016f, m_StepCount);
				m_MainLoopAccumulator -= 0.016f;
            }

			// Camera Update
            float aspectRatio = m_Renderer->GetAspectRatio();
            m_Camera.SetPerspectiveProjection(glm::radians(50.0f), aspectRatio, 0.1f, 10000000.0f);
            m_CameraController.Update(0.016f, m_Camera, m_GameObjects[m_TargetLock]->GetObjectTransform().translation);
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
            m_Renderer->BeginSwapChainRenderPass(commandBuffer, {0.02f, 0.02f, 0.02f});

            m_Renderer->RenderGameObjects(frameInfo);
            m_Renderer->RenderSkybox(frameInfo, m_Skybox.get());
            //m_Renderer->RenderSimpleGeometry(frameInfo, m_Obj.get());

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
    int id = 0;
    ObjectInfo objInfo{};
    objInfo.descriptorPool = m_GlobalPool.get();
    objInfo.device = &m_Device;
    objInfo.sampler = &m_Sampler;

    Properties properties0{};
    properties0.velocity = {0.0f, 0.0f, 0.0f};
    properties0.mass = 5000000;
    properties0.isStatic = false;
    properties0.orbitTraceLenght = 200;
    properties0.rotationSpeed = {0.0f, 0.01f, 0.0f};

    Transform transform0{};
    transform0.translation = {0.0f, 0.0f, 0.0f};
    transform0.scale = glm::vec3{1.0f, 1.0f, 1.0f}*20.0f;
    transform0.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj0 = std::make_unique<Star>(id++, objInfo, "assets/models/smooth_sphere.obj", transform0, properties0, "assets/textures/white.png");
    m_GameObjects.emplace(obj0->GetObjectID(), std::move(obj0));

    Properties properties1{};
    properties1.velocity = {0.0f, 0.0f, -100.0f};
    properties1.mass = 1000;
    properties1.isStatic = false;
    properties1.orbitTraceLenght = 200;
    properties1.rotationSpeed = {0.0f, 0.05f, 0.0f};

    Transform transform1{};
    transform1.translation = {2500.0f, 0.0f, 0.0f};
    transform1.scale = glm::vec3{1.0f, 1.0f, 1.0f} * properties1.mass/200.0f;
    transform1.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj1 = std::make_unique<Planet>(id++, objInfo, "assets/models/smooth_sphere.obj", transform1, properties1, "assets/textures/blue.png");
    m_GameObjects.emplace(obj1->GetObjectID(), std::move(obj1));

    Properties properties2{};
    properties2.velocity = {0.0f, 0.0f, 150.0f};
    properties2.mass = 5;
    properties2.isStatic = false;
    properties2.orbitTraceLenght = 200;
    properties2.rotationSpeed = {0.0f, 0.05f, 0.0f};

    Transform transform2{};
    transform2.translation = {-1000.0f, 0.0f, 0.0f};
    transform2.scale = glm::vec3{1.0f, 1.0f, 1.0f} * properties2.mass/6.0f;
    transform2.rotation = {0.0f, 0.0f, 0.0f};
    std::unique_ptr<Object> obj2 = std::make_unique<Planet>(id++, objInfo, "assets/models/smooth_sphere.obj", transform2, properties2, "assets/textures/red.png");
    m_GameObjects.emplace(obj2->GetObjectID(), std::move(obj2));

    m_Skybox = Skybox::CreateModelFromFile(m_Device, "assets/models/cube.obj");

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

/**
 * @brief "Dummy" function, we need it to pass static function to async because of way we're calculating velocity by now
 */
static void UpdateObj(Object* obj, std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta, uint32_t substeps)
{
    obj->Update(gameObjects, delta, substeps);
}

/**
 * @brief Updates game objects and their orbit traces
 */
void Application::Update(const FrameInfo& frameInfo, float delta, uint32_t substeps)
{
    static int OrbitUpdateCount = 0;
    //std::vector<std::future<void>> futures;
    for (int i = 0; i < m_GameSpeed; i++)
    {
        for (auto& kv: m_GameObjects)
        {
            auto& obj = kv.second;

            const float stepDelta = delta / (float)substeps;
			// If I don't ignore return value it doesn't work properly
            std::async(std::launch::async, UpdateObj, obj.get(), m_GameObjects, stepDelta, substeps);
            //futures.push_back(std::async(std::launch::async, UpdateObj, obj.get(), m_GameObjects, stepDelta, substeps));
            //obj->Update(m_GameObjects, stepDelta, substeps);
            //obj->OrbitUpdate(frameInfo.commandBuffer);
            
            obj->GetObjectTransform().rotation += obj->GetObjectProperties().rotationSpeed;
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

    ImGui::Checkbox("Pause", &m_Pause);

    ImGui::SliderInt("Speed", &m_GameSpeed, 1, 200);
    ImGui::SliderInt("StepCount", &m_StepCount, 1, 2500);
    if (m_FPSaccumulator > 0.5f)
    {
        m_FPS = 1/frameInfo.frameTime;
        m_FPSaccumulator -= 0.5f;
    }
    ImGui::Text("FPS %.1f (%fms)", m_FPS, frameInfo.frameTime);
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
        ImGui::DragFloat((std::to_string(obj->GetObjectID()) + std::string(" Obj Mass")).c_str(), &obj->GetObjectProperties().mass, 500.0f, 1000.0f, 10000000, "%.3f", 0);
    }
    ImGui::Text("Camera Position: x %0.1f y %0.1f z %0.1f", 
        m_Camera.m_Transform.translation.x,
        m_Camera.m_Transform.translation.y,
        m_Camera.m_Transform.translation.z);

    ImGui::End();
 
    ImGui::Render();
    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), frameInfo.commandBuffer);
}