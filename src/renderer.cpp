#include "renderer.h"

#include <stdexcept>
#include <cassert>
#include <array>

Renderer::Renderer(Window& window, Device& device)
    :   m_Window(window), m_Device(device)
{
    CreatePipelineLayout();
    RecreateSwapChain();
    CreateCommandBuffers();
}

Renderer::~Renderer()
{
    FreeCommandBuffers();
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
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
    
    CreatePipeline();
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

    BeginSwapChainRenderPass(commandBuffer);
    return commandBuffer;
}

void Renderer::EndFrame()
{
    auto commandBuffer = GetCurrentCommandBuffer();
    EndSwapChainRenderPass(commandBuffer);
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
}

void Renderer::BeginSwapChainRenderPass(VkCommandBuffer commandBuffer)
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
    clearValues[0].color = {0.02f, 0.02f, 0.02f, 1.0f};
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

void Renderer::RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<std::shared_ptr<Object>> m_GameObjects)
{
    m_Pipeline->Bind(commandBuffer);

    for (auto& obj: m_GameObjects)
    {
        obj->Update();

        PushConstants push{};
        push.transform = obj->GetObjectTransform().mat4();

        vkCmdPushConstants(commandBuffer, m_PipelineLayout, 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 0, sizeof(PushConstants), &push);
        obj->Draw(commandBuffer);
    }
}

void Renderer::CreatePipelineLayout()
{
    VkPushConstantRange pushConstantRange{};
    pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
    pushConstantRange.offset = 0;
    pushConstantRange.size = sizeof(PushConstants);

    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 1;
    pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Renderer::CreatePipeline() 
{
    auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_PipelineLayout;
    m_Pipeline = std::make_unique<Pipeline>(m_Device, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}