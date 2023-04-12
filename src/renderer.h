#pragma once
#include "window.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "object.h"
#include "pipeline.h"
#include "camera.h"
#include "frameInfo.h"
#include "models/simpleModel.h"
#include "skybox.h"

#include <memory>
#include <vector>
#include <cassert>

struct PushConstants
{
    glm::mat4 modelMatrix{1.0f};
};

struct BillboardsPushConstants
{
    glm::vec3 position{0.0f};
};

struct LinesPushConstants
{
    glm::vec3 positions[2];
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
    void RenderBillboards(FrameInfo& frameInfo, glm::vec3 position);
    void RenderOrbits(FrameInfo& frameInfo);
    void RenderSimpleGeometry(FrameInfo& frameInfo, SimpleModel* geometry);
    void RenderSkybox(FrameInfo& frameInfo, Skybox& skybox, VkDescriptorSet skyboxDescriptorSet);
private:
    void CreateCommandBuffers();
    void FreeCommandBuffers();
    void RecreateSwapChain();

    void CreateDefaultPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void CreatePlanetsPipeline();
    void CreateStarsPipeline();

    void CreateBillboardsPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void CreateBillboardsPipeline();

    void CreateLinesPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void CreateLinesPipeline();

    void CreateSimplePipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void CreateSimplePipeline();

    void CreateSkyboxPipelineLayout(VkDescriptorSetLayout globalSetLayout);
    void CreateSkyboxPipeline();

    Window& m_Window;
    Device& m_Device;
    std::unique_ptr<SwapChain> m_SwapChain;
    std::vector<VkCommandBuffer> m_CommandBuffers;

    VkPipelineLayout m_DefaultPipelineLayout;
    std::unique_ptr<Pipeline> m_PlanetsPipeline;
    std::unique_ptr<Pipeline> m_StarsPipeline;

    std::unique_ptr<Pipeline> m_BillboardsPipeline;
    VkPipelineLayout m_BillboardsPipelineLayout;

    std::unique_ptr<Pipeline> m_LinesPipeline;
    VkPipelineLayout m_LinesPipelineLayout;

    std::unique_ptr<Pipeline> m_SimplePipeline;
    VkPipelineLayout m_SimplePipelineLayout;

    std::unique_ptr<Pipeline> m_SkyboxPipeline;
    VkPipelineLayout m_SkyboxPipelineLayout;

    uint32_t m_CurrentImageIndex = 0;
    int m_CurrentFrameIndex = 0;
    bool m_IsFrameStarted = false;
};