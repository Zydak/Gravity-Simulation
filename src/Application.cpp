#include "application.h"

#include <stdexcept>
#include <array>
#include <vector>

Application::Application()
{
    LoadVertexBuffers();
    CreatePipelineLayout();
    RecreateSwapChain();
    CreateCommandBuffers();
}

Application::~Application()
{
    vkDestroyPipelineLayout(m_Device.GetDevice(), m_PipelineLayout, nullptr);
    vkFreeCommandBuffers(m_Device.GetDevice(),m_Device.GetCommandPool(), 
        static_cast<uint32_t>(m_CommandBuffers.size()), m_CommandBuffers.data());
    
    m_CommandBuffers.clear();
}

void Application::Run()
{
    while(!m_Window.ShouldClose())
    {
        glfwPollEvents();
        DrawFrame();
    }

    vkDeviceWaitIdle(m_Device.GetDevice());
}

void Application::CreatePipelineLayout()
{
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = 0;
    pipelineLayoutInfo.pSetLayouts = nullptr;
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    if (vkCreatePipelineLayout(m_Device.GetDevice(), &pipelineLayoutInfo, nullptr, &m_PipelineLayout) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create pipeline layout!");
    }
}

void Application::CreatePipeline() {
    auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo(m_SwapChain->GetWidth(), m_SwapChain->GetHeight());
    pipelineConfig.renderPass = m_SwapChain->GetRenderPass();
    pipelineConfig.pipelineLayout = m_PipelineLayout;
    m_Pipeline = std::make_unique<Pipeline>(m_Device, "shaders/shader.vert.spv", "shaders/shader.frag.spv", pipelineConfig);
}

void Application::CreateCommandBuffers()
{
    m_CommandBuffers.resize(m_SwapChain->GetImageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = m_Device.GetCommandPool();
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_CommandBuffers.size());

    if (vkAllocateCommandBuffers(m_Device.GetDevice(), &allocInfo, m_CommandBuffers.data()) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Application::RecordCommandBuffer(int imageIndex)
{
    VkCommandBufferBeginInfo beginInfo = {};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = 0;                   // Optional
    beginInfo.pInheritanceInfo = nullptr;  // Optional

    if (vkBeginCommandBuffer(m_CommandBuffers[imageIndex], &beginInfo) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    VkRenderPassBeginInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassInfo.renderPass = m_SwapChain->GetRenderPass();
    renderPassInfo.framebuffer = m_SwapChain->GetFrameBuffer(imageIndex);
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_SwapChain->GetSwapChainExtent();

    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.01f, 0.01f, 0.01f, 1.0f};
    clearValues[1].depthStencil = {1.0f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(m_CommandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    m_Pipeline->Bind(m_CommandBuffers[imageIndex]);
    m_Model->Bind(m_CommandBuffers[imageIndex]);
    m_Model->Draw(m_CommandBuffers[imageIndex]);

    vkCmdEndRenderPass(m_CommandBuffers[imageIndex]);
    if (vkEndCommandBuffer(m_CommandBuffers[imageIndex]) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to record command buffer!");
    }
}

void Application::DrawFrame()
{
    uint32_t imageIndex;
    auto result = m_SwapChain->AcquireNextImage(&imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) 
    { 
        RecreateSwapChain(); 
        return; 
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) 
    {
        throw std::runtime_error("failed to acquire swap chain image");
    }

    RecordCommandBuffer(imageIndex);
    result = m_SwapChain->SubmitCommandBuffers(&m_CommandBuffers[imageIndex], &imageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_Window.WasWindowResized()) 
    {
        m_Window.ResetWindowResizedFlag();
        RecreateSwapChain(); 
        return; 
    }
    if (result != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to present swap chain image!");
    }
}

void Application::LoadVertexBuffers()
{
    std::vector<Model::Vertex> vertices
    {
        {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
        {{0.5f, 0.5f},  {0.0f, 1.0f, 0.0f}},
        {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}}
    };

    m_Model = std::make_unique<Model>(m_Device, vertices);
}

void Application::RecreateSwapChain()
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
        m_SwapChain = std::make_unique<SwapChain>(m_Device, extent, std::move(m_SwapChain));
    }
    
    CreatePipeline();
}