#pragma once
#include "vulkan/window.h"
#include "vulkan/device.h"
#include "vulkan/swapchain.h"
#include "vulkan/pipeline.h"
#include "vulkan/skybox.h"
#include "object.h"
#include "camera.h"
#include "frameInfo.h"

#include <memory>
#include <vector>
#include <cassert>

struct PushConstants
{
    glm::mat4 modelMatrix{1.0f};
};

struct BillboardsPushConstants
{
    alignas(16) glm::vec3 position{0.0f};
    alignas(16) glm::vec3 color{1.0f};
    float size = 1.0f;
};

class Renderer
{
public:
    Renderer(Window& window, Device& device, VkDescriptorSetLayout globalSetLayout);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer &operator=(const Renderer&) = delete;

    inline uint32_t GetSwapChainImageCount() { return m_SwapChain->GetImageCount(); }
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
    void RenderOrbits(FrameInfo& frameInfo, Object* obj);
    void RenderSkybox(FrameInfo& frameInfo, Skybox& skybox, VkDescriptorSet skyboxDescriptorSet);
    void RenderBillboards(FrameInfo& frameInfo, glm::vec3 position, float size, glm::vec3 color);
private:
    void CreateCommandBuffers();
    void FreeCommandBuffers();
    void RecreateSwapChain();

    void CreatePipelines();

    void CreatePipelineLayouts(VkDescriptorSetLayout globalSetLayout);

    Window& m_Window;
    Device& m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    VkPipelineLayout m_DefaultPipelineLayout;
    std::unique_ptr<Pipeline> m_PlanetsPipeline;
    std::unique_ptr<Pipeline> m_StarsPipeline;

    std::unique_ptr<Pipeline> m_OrbitsPipeline;
    VkPipelineLayout m_OrbitsPipelineLayout;

    std::unique_ptr<Pipeline> m_SkyboxPipeline;
    VkPipelineLayout m_SkyboxPipelineLayout;

    std::unique_ptr<Pipeline> m_BillboardPipeline;
    VkPipelineLayout m_BillboardPipelineLayout;

    uint32_t m_CurrentImageIndex = 0;
    int m_CurrentFrameIndex = 0;
    bool m_IsFrameStarted = false;
};