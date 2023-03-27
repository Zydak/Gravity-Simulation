#pragma once
#include "window.h"
#include "pipeline.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "model.h"

#include <memory>
#include <vector>

class Application
{
public:
    Application();
    ~Application();

    void Run();
private:
    void LoadVertexBuffers();
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void DrawFrame();

    Window m_Window{800, 600, "Gravity"};
    Device m_Device{m_Window};
    SwapChain m_SwapChain{m_Device, m_Window.GetExtent()};
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::unique_ptr<Model> m_Model;
};