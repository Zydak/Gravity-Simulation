#pragma once

#include <vulkan/vulkan.h>

#include <vector>

struct QueueFamilyIndices 
{
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool IsComplete() { return graphicsFamilyHasValue; //&& presentFamilyHasValue; }
    }
};

class Device
{
public:
    Device();
    ~Device();

private:
    std::vector<const char *> GetRequiredGlfwExtensions();
    void CheckRequiredGlfwExtensions();
    bool IsDeviceSuitable(VkPhysicalDevice device);
    QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

    void CreateInstance();
    void SetupDebugMessenger();
    void PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);
    void PickPhysicalDevice();

    bool CheckValidationLayerSupport();

    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;
    VkPhysicalDevice m_PhysicalDevice;

    const std::vector<const char *> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool m_EnableValidationLayers = false;
#else
    const bool m_EnableValidationLayers = true;
#endif
};