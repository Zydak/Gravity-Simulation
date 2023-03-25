#include "renderer.h"

Renderer::Renderer(Window& window, Device& device)
    : m_Window(window), m_Device(device)
{
    RecreateSwapChain();
    m_Pipeline = std::make_unique<Pipeline>(m_Device, "shaders/shader.vert.spv", "shaders/shader.frag.spv", Pipeline::DefaultPipelineConfigInfo());
}

Renderer::~Renderer()
{

}

void Renderer::RecreateSwapChain()
{
    auto extent = m_Window.GetExtent();
    while (extent.width == 0 || extent.height == 0)
    {
        extent = m_Window.GetExtent();
        glfwWaitEvents();
    }
    m_SwapChain = std::make_unique<SwapChain>(m_Device, extent);
}