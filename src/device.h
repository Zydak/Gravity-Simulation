#pragma once

#include <vulkan/vulkan.h>

#include "window.h"

#include <vector>

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

private:
    std::vector<const char *> GetRequiredGlfwExtensions();
    void CheckRequiredGlfwExtensions();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    void CreateInstance();
    void SetupDebugMessenger();
    void PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void PickPhysicalDevice();
    void CreateLogicalDevice();
    void CreateSurface();

    bool CheckValidationLayerSupport();

    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice;
    VkDevice m_Device;
    VkSurfaceKHR  m_Surface;
    Window &m_Window;

    VkQueue m_GraphicsQueue;
    VkQueue m_PresentQueue;

    const std::vector<const char *> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool m_EnableValidationLayers = false;
#else
    const bool m_EnableValidationLayers = true;
#endif
};