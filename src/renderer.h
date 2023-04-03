#pragma once
#include "window.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "object.h"
#include "pipeline.h"
#include "camera.h"
#include "frameInfo.h"

#include <memory>
#include <vector>
#include <cassert>

struct PushConstants
{
    glm::mat4 modelMatrix{1.0f};
};

class Renderer
{
public:
    Renderer(Window& window, Device& device, VkDescriptorSetLayout globalSetLayout);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer &operator=(const Renderer&) = delete;

    inline VkRenderPass GetSwapChainRenderPass() { return m_SwapChain->GetRenderPass(); }
    inline float GetAspectRatio() { return m_SwapChain->GetExtentAspectRatio(); }
    inline bool IsFrameInProgress() const { return m_IsFrameStarted; }
    VkCommandBuffer GetCurrentCommandBuffer() const
    {
        assert(m_IsFrameStarted && "Cannot get command buffer when frame is not in progress");
        return m_CommandBuffers[m_CurrentImageIndex];
    }
    int GetFrameIndex() const
    {
        assert(m_IsFrameStarted && "Cannot get frame index when frameis not in progress");
        return m_CurrentFrameIndex;
    }

    VkCommandBuffer BeginFrame(const glm::vec3& clearColor);
    void EndFrame();
    void BeginSwapChainRenderPass(VkCommandBuffer commandBuffer, const glm::vec3& clearColor);
    void EndSwapChainRenderPass(VkCommandBuffer commandBuffer);
    void RenderGameObjects(FrameInfo& frameInfo);
private:
    void CreateCommandBuffers();
    void FreeCommandBuffers();
    void RecreateSwapChain();
    void CreatePipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void CreatePipeline();

    Window& m_Window;
    Device& m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::vector<VkCommandBuffer> m_CommandBuffers;
    std::unique_ptr<Pipeline> m_Pipeline;
    VkPipelineLayout m_PipelineLayout;

    uint32_t m_CurrentImageIndex = 0;
    int m_CurrentFrameIndex = 0;
    bool m_IsFrameStarted = false;
};