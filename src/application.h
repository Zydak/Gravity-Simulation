#pragma once
#include "window.h"
#include "pipeline.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "object.h"
#include "renderer.h"
#include "camera.h"
#include "cameraController.h"

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

class Application
{
public:
    Application();
    ~Application();

    void Run();
private:
    void LoadGameObjects();
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void DrawFrame();
    void RecreateSwapChain();
    void RecordCommandBuffer(int imageIndex);
    void RenderGameObjects(VkCommandBuffer commandBuffer);

    Window m_Window{800, 600, "Gravity"};
    Device m_Device{m_Window};
    Renderer m_Renderer{m_Window, m_Device};
    Camera m_Camera{};
    CameraController m_CameraController{m_Window.GetGLFWwindow()};
    std::unique_ptr<SwapChain> m_SwapChain;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::vector<std::shared_ptr<Object>> m_GameObjects;
};