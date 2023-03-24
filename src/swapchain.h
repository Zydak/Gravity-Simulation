#pragma once

#include "device.h"

#include <vulkan/vulkan.h>

#include <vector>
#include <memory>

class SwapChain
{
public:
    SwapChain(Device &deviceRef, VkExtent2D windowExtent);
    SwapChain(Device &deviceRef, VkExtent2D windowExtent, std::shared_ptr<SwapChain> previous);
    ~SwapChain();

    SwapChain(const SwapChain &) = delete;
    SwapChain& operator=(const SwapChain &) = delete;

private:
    void CreateSwapChain();
    void CreateImageViews();

    VkSurfaceFormatKHR ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR> &availableFormats);
    VkPresentModeKHR ChooseSwapPresentMode(const std::vector<VkPresentModeKHR> &availablePresentModes);
    VkExtent2D ChooseSwapExtent(const VkSurfaceCapabilitiesKHR &capabilities);

    VkSwapchainKHR m_SwapChain;
    Device& m_Device;
    VkExtent2D m_WindowExtent;

    std::vector<VkImage> m_SwapChainImages;
    std::vector<VkImageView> m_SwapChainImageViews;

    VkFormat m_SwapChainImageFormat;
    VkExtent2D m_SwapChainExtent; 

};