#include "renderer.h"

#include "models/customModel.h"

#include <stdexcept>
#include <cassert>
#include <array>

Renderer::Renderer(Window& window, Device& device, VkDescriptorSetLayout globalSetLayout)
    :   m_Window(window), m_Device(device)
{
    CreatePipelineLayouts(globalSetLayout);
    RecreateSwapChain();
    CreateCommandBuffers();
}

Renderer::~Renderer()
{
    FreeCommandBuffers();
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_DefaultPipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_OrbitsPipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_SkyboxPipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_BillboardPipelineLayout, nullptr);
    m_CommandBuffers.clear();
}

void Renderer::RecreateSwapChain()
{
    auto extent = m_Window.GetExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = m_Window.GetExtent();
        glfwWaitEvents();
    }
    vkDeviceWaitIdle(m_Device.GetDevice());

    if(m_SwapChain == nullptr)
    {
        m_SwapChain = std::make_unique<SwapChain>(m_Device, extent);
    }
    else
    {
        std::shared_ptr<SwapChain> oldSwapChain = std::move(m_SwapChain);
        m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, oldSwapChain);
    }
    
    CreatePipelines();
}

void Renderer::CreateCommandBuffers()
{
    m_CommandBuffers.resize(SwapChain::MAX_FRAMES_IN_FLIGHT);

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_Device.GetCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());
    if (vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::FreeCommandBuffers()
{
    vkFreeCommandBuffers(
        m_Device.GetDevice(),
        m_Device.GetCommandPool(),
        static_cast<float>(m_CommandBuffers.size()),
        m_CommandBuffers.data()
    );
    m_CommandBuffers.clear();
}

VkCommandBuffer Renderer::BeginFrame()
{
    assert(!m_IsFrameStarted && "Can't call BeginFrame while already in progress!");

    auto result = m_SwapChain->AcquireNextImage(&m_CurrentImageIndex); 
    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    {
        RecreateSwapChain();
        return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    m_IsFrameStarted = true;
    auto commandBuffer = GetCurrentCommandBuffer();

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }
    return commandBuffer;
}

void Renderer::EndFrame()
{
    auto commandBuffer = GetCurrentCommandBuffer();
    assert(m_IsFrameStarted && "Can't call EndFrame while frame is not in progress");

    if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = m_SwapChain->SubmitCommandBuffers(&commandBuffer, &m_CurrentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasWindowResized())
    {
        m_Window.ResetWindowResizedFlag();
        RecreateSwapChain();
    }
    else if (result != VK_SUCCESS)
    {
        throw std::runtime_error("failed to present swap chain image!");
    }

    m_IsFrameStarted = false;
    m_CurrentFrameIndex = (m_CurrentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
}

void Renderer::BeginGeometryRenderPass(VkCommandBuffer commandBuffer, const glm::vec3& clearColor)
{
    assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't Begin Render pass on command buffer from different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->GetSwapchainFrameBuffer(m_CurrentImageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {clearColor.r, clearColor.g, clearColor.b};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();
    
    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    VkViewport viewport{};
    viewport.x = 0.0f;
    viewport.y = 0.0f;
    viewport.width = static_cast<float>(m_SwapChain->GetSwapChainExtent().width);
    viewport.height = static_cast<float>(m_SwapChain->GetSwapChainExtent().height);
    viewport.minDepth = 0.0f;
    viewport.maxDepth = 1.0f;
    VkRect2D scissor{{0, 0}, m_SwapChain->GetSwapChainExtent()};
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
}

void Renderer::EndGeometryRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_IsFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from different frame");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::RenderOrbits(FrameInfo& frameInfo, Object* obj)
{
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_OrbitsPipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr
    );
    
    m_OrbitsPipeline->Bind(frameInfo.commandBuffer);

    OrbitPushConstants push{};
    push.offset = frameInfo.offset/SCALE_DOWN;
    push.color = obj->GetObjectColor();

    vkCmdPushConstants(frameInfo.commandBuffer, m_OrbitsPipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(OrbitPushConstants), &push);

    obj->DrawOrbit(frameInfo.commandBuffer);
}

void Renderer::RenderGameObjects(FrameInfo& frameInfo)
{
    for (auto& kv: frameInfo.gameObjects)
    {
        auto& obj = kv.second;
        auto offset = ((frameInfo.camera->m_Transform.translation*SCALE_DOWN)+frameInfo.offset) - (obj->GetObjectTransform().translation);
        double distance = std::sqrt(glm::dot(offset, offset));
        if (distance < obj->GetObjectProperties().radius*(SCALE_DOWN/1000000))
        {
            vkCmdBindDescriptorSets(
                frameInfo.commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                m_DefaultPipelineLayout,
                0,
                1,
                &frameInfo.globalDescriptorSet,
                0,
                nullptr
            );

            if (obj->GetObjectType() == OBJ_TYPE_PLANET)
                m_PlanetsPipeline->Bind(frameInfo.commandBuffer);
            if (obj->GetObjectType() == OBJ_TYPE_STAR)
                m_StarsPipeline->Bind(frameInfo.commandBuffer);

            PushConstants push{};
            push.modelMatrix = obj->GetObjectTransform().mat4();
            push.offset = frameInfo.offset/SCALE_DOWN;

            vkCmdPushConstants(frameInfo.commandBuffer, m_DefaultPipelineLayout, 
                VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &push);
            obj->Draw(m_DefaultPipelineLayout, frameInfo.commandBuffer);
        }
        else
        {
            RenderOrbits(frameInfo, obj.get());
            RenderBillboards(frameInfo, (obj->GetObjectTransform().translation)/SCALE_DOWN, distance*2/(SCALE_DOWN*100), obj->GetObjectColor());
        }
    }
}

void Renderer::RenderBillboards(FrameInfo& frameInfo, glm::vec3 position, float size, glm::vec3 color)
{
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_BillboardPipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr
    );

    m_BillboardPipeline->Bind(frameInfo.commandBuffer);

    BillboardsPushConstants push{};
    push.position = position;
    push.size = size;
    push.color = color;
    push.offset = frameInfo.offset/SCALE_DOWN;

    vkCmdPushConstants(frameInfo.commandBuffer, m_BillboardPipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(BillboardsPushConstants), &push);

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

void Renderer::RenderSkybox(FrameInfo& frameInfo, Skybox& skybox, VkDescriptorSet skyboxDescriptorSet)
{
    m_SkyboxPipeline->Bind(frameInfo.commandBuffer);

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_SkyboxPipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr
    );

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_SkyboxPipelineLayout,
        1,
        1,
        &skyboxDescriptorSet,
        0,
        nullptr
    );
    
    
    glm::dvec3 translation = frameInfo.camera->m_Transform.translation;
    glm::dvec3 scale = glm::vec3{1.0f, 1.0f, 1.0f} * 500.0f;
    auto transform = glm::translate(glm::dmat4{1.0f}, translation);
    transform = glm::scale(transform, scale);

    PushConstants push{};
    push.modelMatrix = transform;

    vkCmdPushConstants(frameInfo.commandBuffer, m_SkyboxPipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(PushConstants), &push);
    
    skybox.GetSkyboxModel()->Bind(frameInfo.commandBuffer);
    skybox.GetSkyboxModel()->Draw(frameInfo.commandBuffer);
}

void Renderer::CreatePipelineLayouts(VkDescriptorSetLayout globalSetLayout)
{
    //
    // DEFAULT
    //
    // stars and planets have the same pipeline layout for now
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);

        auto m_SetLayout = DescriptorSetLayout::Builder(m_Device)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, m_SetLayout->GetDescriptorSetLayout()};

        Pipeline::CreatePipelineLayout(m_Device, descriptorSetLayouts, m_DefaultPipelineLayout, &pushConstantRange);
    }

    //
    // ORBITS
    //
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(OrbitPushConstants);

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        Pipeline::CreatePipelineLayout(m_Device, descriptorSetLayouts, m_OrbitsPipelineLayout, &pushConstantRange);
    }

    //
    // SKYBOX
    //
    {
        auto m_SkyboxLayout = DescriptorSetLayout::Builder(m_Device)
            .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .Build();

        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, m_SkyboxLayout->GetDescriptorSetLayout()};

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstants);

        Pipeline::CreatePipelineLayout(m_Device, descriptorSetLayouts, m_SkyboxPipelineLayout, &pushConstantRange);
    }

    //
    // Billboards
    //
    {
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(BillboardsPushConstants);

        Pipeline::CreatePipelineLayout(m_Device, descriptorSetLayouts, m_BillboardPipelineLayout, &pushConstantRange);
    }
}

void Renderer::CreatePipelines()
{
    //
    // PLANETS
    //
    {
        auto pipelineConfig = Pipeline::CreatePipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight(),
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_CULL_MODE_FRONT_BIT, true, false
        );
        pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
        pipelineConfig.pipelineLayout = m_DefaultPipelineLayout;
        m_PlanetsPipeline = std::make_unique<Pipeline>(m_Device);
        m_PlanetsPipeline->CreatePipeline("../shaders/spv/sphere.vert.spv", "../shaders/spv/sphere.frag.spv", 
            pipelineConfig,
            CustomModel::Vertex::GetBindingDescriptions(),
            CustomModel::Vertex::GetAttributeDescriptions()
        );
    }

    //
    // STARS
    //
    {
        auto pipelineConfig = Pipeline::CreatePipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight(),
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_CULL_MODE_FRONT_BIT, true, false
        );
        pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
        pipelineConfig.pipelineLayout = m_DefaultPipelineLayout;
        m_StarsPipeline = std::make_unique<Pipeline>(m_Device);
        m_StarsPipeline->CreatePipeline("../shaders/spv/stars.vert.spv", "../shaders/spv/stars.frag.spv", 
            pipelineConfig,
            CustomModel::Vertex::GetBindingDescriptions(),
            CustomModel::Vertex::GetAttributeDescriptions()
        );
    }

    //
    // ORBITS
    //
    {
        auto pipelineConfig = Pipeline::CreatePipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight(),
            VK_PRIMITIVE_TOPOLOGY_LINE_STRIP,
            VK_CULL_MODE_NONE, false, false
        );
        pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
        pipelineConfig.pipelineLayout = m_OrbitsPipelineLayout;
        m_OrbitsPipeline = std::make_unique<Pipeline>(m_Device);
        m_OrbitsPipeline->CreatePipeline("../shaders/spv/orbits.vert.spv", "../shaders/spv/orbits.frag.spv",
            pipelineConfig,
            CustomModelPosOnly::Vertex::GetBindingDescriptions(),
            CustomModelPosOnly::Vertex::GetAttributeDescriptions()
        );
    }

    //
    // SKYBOX
    //
    {
        auto pipelineConfig = Pipeline::CreatePipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight(),
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_CULL_MODE_BACK_BIT, false, false
        );
        pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
        pipelineConfig.pipelineLayout = m_SkyboxPipelineLayout;
        m_SkyboxPipeline = std::make_unique<Pipeline>(m_Device);
        m_SkyboxPipeline->CreatePipeline("../shaders/spv/skybox.vert.spv", "../shaders/spv/skybox.frag.spv", 
            pipelineConfig,
            CustomModelPosOnly::Vertex::GetBindingDescriptions(),
            CustomModelPosOnly::Vertex::GetAttributeDescriptions()
        );
    }

    //
    // Billboards
    //
    {
        auto pipelineConfig = Pipeline::CreatePipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight(),
            VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
            VK_CULL_MODE_FRONT_BIT, false, true
        );
        pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
        pipelineConfig.pipelineLayout = m_BillboardPipelineLayout;
        m_BillboardPipeline = std::make_unique<Pipeline>(m_Device);
        m_BillboardPipeline->CreatePipeline("../shaders/spv/billboard.vert.spv", "../shaders/spv/billboard.frag.spv", 
            pipelineConfig
        );
    }
}