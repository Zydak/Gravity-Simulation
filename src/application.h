#pragma once
#include "window.h"
#include "pipeline.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"

#include <memory>
#include <vector>

class Application
{
public:
    Application();
    ~Application();

    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 800;

    void Run();
private:
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void DrawFrame();

    Window m_Window{WIDTH, HEIGHT, "Gravity"};
    Device m_Device{m_Window};
    SwapChain m_SwapChain{m_Device, m_Window.GetExtent()};
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
    std::vector<VkCommandBuffer> m_CommandBuffers;
};