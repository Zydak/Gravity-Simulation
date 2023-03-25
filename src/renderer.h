#pragma once

#include "window.h"
#include "device.h"
#include "swapchain.h"
#include "pipeline.h"

#include <memory>

class Renderer
{
public:
    Renderer(Window& window, Device& device);
    ~Renderer();

    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 600;

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void RecreateSwapChain();

private:
    std::unique_ptr<SwapChain> m_SwapChain;
    Window &m_Window;
    Device &m_Device;
    
    VkPipelineLayout m_PipelineLayout;
    std::unique_ptr<Pipeline> m_Pipeline;
};