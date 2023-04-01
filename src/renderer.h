#pragma once
#include "window.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "object.h"
#include "pipeline.h"

#include <memory>
#include <vector>
#include <cassert>

struct PushConstants
{
    glm::mat4 transform{1.0f};
};

class Renderer
{
public:
    Renderer(Window& m_Window, Device& m_Device);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer &operator=(const Renderer&) = delete;

    inline VkRenderPass GetSwapChainRenderPass() { return m_SwapChain->GetRenderPass(); }
    inline bool IsFrameInProgress() const { return m_IsFrameStarted; }
    VkCommandBuffer GetCurrentCommandBuffer() const
    {
        assert(m_IsFrameStarted && "Cannot get command buffer when frame is not in progress");
        return m_CommandBuffers[m_CurrentImageIndex];
    }

    VkCommandBuffer BeginFrame();
    void EndFrame();
    void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void RenderGameObjects(VkCommandBuffer commandBuffer, std::vector<std::shared_ptr<Object>> m_GameObjects);
private:
    void CreateCommandBuffers();
    void FreeCommandBuffers();
    void RecreateSwapChain();
    void CreatePipelineLayout();
    void CreatePipeline();

    Window& m_Window;
    Device& m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;

    uint32_t m_CurrentImageIndex = 0;
    bool m_IsFrameStarted = false;
};