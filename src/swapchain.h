#pragma once

#include "device.h"

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

    VkRenderPass GetRenderPass() { return m_RenderPass; }
    VkFramebuffer GetFrameBuffer(int index) { return m_SwapChainFramebuffers[index]; }
    uint32_t GetWidth() { return m_SwapChainExtent.width; }
    uint32_t GetHeight() { return m_SwapChainExtent.height; }
    VkFormat GetSwapChainImageFormat() { return m_SwapChainImageFormat; }
    size_t GetImageCount() { return m_SwapChainImageViews.size(); }
    VkExtent2D GetSwapChainExtent() { return m_SwapChainExtent; }

    VkResult SubmitCommandBuffers(const VkCommandBuffer *buffers, uint32_t *imageIndex);
    VkResult AcquireNextImage(uint32_t *imageIndex);
    bool CompareSwapFormats(const SwapChain& swapChain) const
    {
        return swapChain.m_SwapChainDepthFormat == m_SwapChainDepthFormat && swapChain.m_SwapChainImageFormat == m_SwapChainImageFormat;
    }

private:
    void Init();
    void CreateSwapChain();
    void CreateImageViews();
    void CreateDepthResources();
    void CreateRenderPass();
    void CreateFramebuffers();
    void CreateSyncObjects();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);
    VkFormat FindDepthFormat();
    
    std::shared_ptr<SwapChain> m_OldSwapChain;
    VkSwapchainKHR m_SwapChain;
    Device& m_Device;
    VkExtent2D m_WindowExtent;

    size_t m_CurrentFrame = 0;
    std::vector<VkFramebuffer> m_SwapChainFramebuffers;
    
    std::vector<VkImage> m_DepthImages;
    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkDeviceMemory> m_DepthImageMemories;
    std::vector<VkImageView> m_SwapChainImageViews;
    std::vector<VkImageView> m_DepthImageViews;
    
    std::vector<VkSemaphore> m_ImageAvailableSemaphores;
    std::vector<VkSemaphore> m_RenderFinishedSemaphores;
    std::vector<VkFence> m_InFlightFences;
    std::vector<VkFence> m_ImagesInFlight;

    VkFormat m_SwapChainImageFormat;
    VkFormat m_SwapChainDepthFormat;
    VkExtent2D m_SwapChainExtent; 

    VkRenderPass m_RenderPass;
};