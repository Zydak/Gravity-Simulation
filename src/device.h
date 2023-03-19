#pragma once

#include <vulkan/vulkan.h>

#include <vector>

class Device
{
public:
    Device();
    ~Device();

private:
    std::vector<const char *> GetRequiredGlfwExtensions();
    void CheckRequiredGlfwExtensions();

    void CreateInstance();
    void SetupDebugMessenger();
    void PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

    bool CheckValidationLayerSupport();

    VkInstance m_Instance;
    VkDebugUtilsMessengerEXT m_DebugMessenger;

    const std::vector<const char *> m_ValidationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
    const bool m_EnableValidationLayers = false;
#else
    const bool m_EnableValidationLayers = true;
#endif
};