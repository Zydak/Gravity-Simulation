#include "renderer.h"

#include <stdexcept>
#include <cassert>
#include <array>

Renderer::Renderer(Window& window, Device& device, VkDescriptorSetLayout globalSetLayout)
    :   m_Window(window), m_Device(device)
{
    CreateDefaultPipelineLayout(globalSetLayout);
    CreateLinesPipelineLayout(globalSetLayout);
    CreateSimplePipelineLayout(globalSetLayout);
    CreateSkyboxPipelineLayout(globalSetLayout);
    //CreateBillboardsPipelineLayout(globalSetLayout);
    RecreateSwapChain();
    CreateCommandBuffers();
}

Renderer::~Renderer()
{
    FreeCommandBuffers();
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_DefaultPipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_LinesPipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_SimplePipelineLayout, nullptr);
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_SkyboxPipelineLayout, nullptr);
    //vkDestroyPipelineLayout(m_Device.GetDevice(), m_BillboardsPipelineLayout, nullptr);
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
        if(!oldSwapChain->CompareSwapFormats(*m_SwapChain.get()))
        {
            throw std::runtime_error("Swap chain image or depth formats have changed!");
        }
    }
    
    CreatePlanetsPipeline();
    CreateStarsPipeline();
    CreateLinesPipeline();
    CreateSimplePipeline();
    CreateSkyboxPipeline();
    //CreateBillboardsPipeline();
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

VkCommandBuffer Renderer::BeginFrame(const glm::vec3& clearColor)
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

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, const glm::vec3& clearColor)
{
    assert(m_IsFrameStarted && "Can't call BeginSwapChainRenderPass while frame is not in progress");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't Begin Render pass on command buffer from different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(m_CurrentImageIndex);
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

void Renderer::EndSwapChainRenderPass(VkCommandBuffer commandBuffer)
{
    assert(m_IsFrameStarted && "Can't call EndSwapChainRenderPass while frame is not in progress");
    assert(commandBuffer == GetCurrentCommandBuffer() && "Can't end render pass on command buffer from different frame");

    vkCmdEndRenderPass(commandBuffer);
}

void Renderer::RenderOrbits(FrameInfo& frameInfo)
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

    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_LinesPipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr
    );

    m_LinesPipeline->Bind(frameInfo.commandBuffer);

    for (auto& kv: frameInfo.gameObjects)
    {
        auto& obj = kv.second;
        if (obj->GetObjectProperties().orbitTraceLenght <= 0)
            continue;

        obj->DrawOrbit(frameInfo.commandBuffer);
    }
}

void Renderer::RenderGameObjects(FrameInfo& frameInfo)
{
    RenderOrbits(frameInfo);

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

    for (auto& kv: frameInfo.gameObjects)
    {
        auto& obj = kv.second;
        if (obj->GetObjectType() == OBJ_TYPE_PLANET)
            m_PlanetsPipeline->Bind(frameInfo.commandBuffer);
        if (obj->GetObjectType() == OBJ_TYPE_STAR)
            m_StarsPipeline->Bind(frameInfo.commandBuffer);

        PushConstants push{};
        push.modelMatrix = obj->GetObjectTransform().mat4();

        vkCmdPushConstants(frameInfo.commandBuffer, m_DefaultPipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push);
        obj->Draw(m_DefaultPipelineLayout, frameInfo.commandBuffer);
    }
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
    
    
    glm::vec3 translation = frameInfo.camera->m_Transform.translation;
    glm::vec3 rotation = {0.0f, 0.0f, 0.0f};
    glm::vec3 scale = glm::vec3{1.0f, 1.0f, 1.0f} * 100000.0f;
    auto transform = glm::translate(glm::mat4{1.0f}, translation);

    transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
    transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
    transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.0f});
    transform = glm::scale(transform, scale);

    PushConstants push{};
    push.modelMatrix = transform;

    vkCmdPushConstants(frameInfo.commandBuffer, m_SkyboxPipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push);
    
    skybox.GetSkyboxModel()->Bind(frameInfo.commandBuffer);
    skybox.GetSkyboxModel()->Draw(frameInfo.commandBuffer);
}

void Renderer::RenderSimpleGeometry(FrameInfo& frameInfo, SimpleModel* geometry)
{
    vkCmdBindDescriptorSets(
        frameInfo.commandBuffer,
        VK_PIPELINE_BIND_POINT_GRAPHICS,
        m_SimplePipelineLayout,
        0,
        1,
        &frameInfo.globalDescriptorSet,
        0,
        nullptr
    );

    m_SimplePipeline->Bind(frameInfo.commandBuffer);
    geometry->Bind(frameInfo.commandBuffer);
    geometry->Draw(frameInfo.commandBuffer);
}

void Renderer::CreateDefaultPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    auto m_SetLayout = DescriptorSetLayout::Builder(m_Device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, m_SetLayout->GetDescriptorSetLayout()};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_DefaultPipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Renderer::CreatePlanetsPipeline() 
{
    auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_DefaultPipelineLayout;
    m_PlanetsPipeline = std::make_unique<Pipeline>(m_Device);
    m_PlanetsPipeline->CreateGraphicsPipeline("shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}

void Renderer::CreateStarsPipeline() 
{
    auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_DefaultPipelineLayout;
    m_StarsPipeline = std::make_unique<Pipeline>(m_Device);
    m_StarsPipeline->CreateGraphicsPipeline("shaders/stars.vert.spv", "shaders/stars.frag.spv", pipelineConfig);
}

void Renderer::CreateBillboardsPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(BillboardsPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_BillboardsPipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Renderer::CreateBillboardsPipeline() 
{
    auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_BillboardsPipelineLayout;
    m_BillboardsPipeline = std::make_unique<Pipeline>(m_Device);
    m_BillboardsPipeline->CreateGraphicsPipeline("shaders/billboard.vert.spv", "shaders/billboard.frag.spv", pipelineConfig);
}

void Renderer::CreateLinesPipeline() 
{
    auto pipelineConfig = Pipeline::LinesPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_LinesPipelineLayout;
    m_LinesPipeline = std::make_unique<Pipeline>(m_Device);
    m_LinesPipeline->CreateLinesPipeline("shaders/lines.vert.spv", "shaders/lines.frag.spv", pipelineConfig);
}
void Renderer::CreateLinesPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(LinesPushConstants);

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_LinesPipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Renderer::RenderBillboards(FrameInfo& frameInfo, glm::vec3 position)
{
    m_BillboardsPipeline->Bind(frameInfo.commandBuffer);

    BillboardsPushConstants push{};
    push.position = position;

    vkCmdPushConstants(frameInfo.commandBuffer, m_DefaultPipelineLayout, 
        VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push);

    vkCmdDraw(frameInfo.commandBuffer, 6, 1, 0, 0);
}

void Renderer::CreateSimplePipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout};

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_SimplePipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Renderer::CreateSimplePipeline()
{
    auto pipelineConfig = Pipeline::SimplePipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_SimplePipelineLayout;
    m_SimplePipeline = std::make_unique<Pipeline>(m_Device);
    m_SimplePipeline->CreateSimplePipeline("shaders/simple.vert.spv", "shaders/simple.frag.spv", pipelineConfig);
}

void Renderer::CreateSkyboxPipelineLayout(VkDescriptorSetLayout globalSetLayout)
{
    auto m_SkyboxLayout = DescriptorSetLayout::Builder(m_Device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();

    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{globalSetLayout, m_SkyboxLayout->GetDescriptorSetLayout()};

    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = (uint32_t)descriptorSetLayouts.size();
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_SkyboxPipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Renderer::CreateSkyboxPipeline()
{
    auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_SkyboxPipelineLayout;
    m_SkyboxPipeline = std::make_unique<Pipeline>(m_Device);
    m_SkyboxPipeline->CreateSkyboxPipeline("shaders/skybox.vert.spv", "shaders/skybox.frag.spv", pipelineConfig);
}