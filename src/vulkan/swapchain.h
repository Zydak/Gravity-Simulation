#pragma once

#include "device.h"
#include "renderPass.h"
#include "framebuffer.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

class SwapChain
{
public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previousSwapChain);
    ~SwapChain();

    SwapChain(const SwapChain &) = delete;
    SwapChain& operator=(const SwapChain &) = delete;

    VkRenderPass GetRenderPass() { return m_RenderPass->GetRenderPass(); }
    VkFramebuffer GetSwapchainFrameBuffer(int index) { return m_SwapChainFramebuffers->GetFramebuffer(index); }
    uint32_t GetWidth() { return m_SwapChainExtent.width; }
    uint32_t GetHeight() { return m_SwapChainExtent.height; }
    VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; }
    size_t GetImageCount() { return imageCount; }
    VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }

    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
    VkResult AcquireNextImage(uint32_t *imageIndex);
    float GetExtentAspectRatio() 
    {
        return static_cast<float>(m_SwapChainExtent.width) / static_cast<float>(m_SwapChainExtent.height); 
    }

    static VkFormat FindDepthFormat(Device& device);
    static VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
private:
    void CreateSwapChain();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateSyncObjects();

    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    
    std::shared_ptr<SwapChain> m_OldSwapChain;
    VkSwapchainKHR m_SwapChain;
    Device& m_Device;
    VkExtent2D m_WindowExtent;

    uint32_t imageCount;
    size_t m_CurrentFrame = 0;
    std::unique_ptr<Framebuffer> m_SwapChainFramebuffers;
    
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;

    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent; 

    std::unique_ptr<RenderPass> m_RenderPass;
};