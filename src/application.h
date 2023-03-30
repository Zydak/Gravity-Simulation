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
    void RecreateSwapChain();
    void RecordCommandBuffer(int imageIndex);

    Window m_Window{800, 600, "Gravity"};
    Device m_Device{m_Window};
    std::unique_ptr<SwapChain> m_SwapChain;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::unique_ptr<Model> m_Model;
};