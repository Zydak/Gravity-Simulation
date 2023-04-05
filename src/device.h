#pragma once

#include <vulkan/vulkan.h>

#include "window.h"

#include <vector>

struct SwapChainSupportDetails 
{
  VkSurfaceCapabilitiesKHR capabilities; // min/max number of images
  std::vector<VkSurfaceFormatKHR> formats; // pixel format, color space
  std::vector<VkPresentModeKHR> presentModes;
};

struct QueueFamilyIndices 
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool IsComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

class Device
{
public:
    Device(Window &window);
    ~Device();

    Device(const Device &) = delete;
    Device& operator=(const Device &) = delete;
    Device(Device &&) = delete;
    Device &operator=(Device &&) = delete;

    inline VkInstance GetInstance() { return m_Instance; }
    inline VkDevice GetDevice() { return m_Device; }
    inline VkPhysicalDevice GetPhysicalDevice() { return m_PhysicalDevice; }
    inline SwapChainSupportDetails GetSwapChainSupport() { return QuerySwapChainSupport(m_PhysicalDevice); }
    inline VkSurfaceKHR GetSurface() { return m_Surface; }
    inline QueueFamilyIndices FindPhysicalQueueFamilies() { return FindQueueFamilies(m_PhysicalDevice); }
    inline VkCommandPool GetCommandPool() { return m_CommandPool; }
    inline VkQueue GetGraphicsQueue() { return m_GraphicsQueue; }
    inline VkQueue GetPresentQueue() { return m_PresentQueue; }

    VkFormat FindSupportedFormat(const std::vector<VkFormat> &candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
    void CreateImageWithInfo(const VkImageCreateInfo &imageInfo, VkMemoryPropertyFlags properties, VkImage &image, VkDeviceMemory &imageMemory);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer &buffer, VkDeviceMemory &bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);

    VkPhysicalDeviceProperties m_Properties;
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    void BeginSingleTimeCommands(VkCommandBuffer& buffer);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
private:
    std::vector<const char *> GetRequiredGlfwExtensions();
    void CheckRequiredGlfwExtensions();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    void CreateInstance();
    void SetupDebugMessenger();
    void CreateSurface();
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateCommandPool();

    void PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    bool CheckValidationLayerSupport();
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    SwapChainSupportDetails QuerySwapChainSupport(VkPhysicalDevice device);
    

    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR  m_Surface;
    Window &m_Window;

    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;

    VkCommandPool m_CommandPool;

    const std::vector<const char *> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char *> m_DeviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};

#ifdef NDEBUG
    const bool m_EnableValidationLayers = false;
#else
    const bool m_EnableValidationLayers = true;
#endif
};