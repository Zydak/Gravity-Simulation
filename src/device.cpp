#include "device.h"
#include "GLFW/glfw3.h"

#include <stdexcept>
#include <iostream>
#include <unordered_set>
#include <cstring>

/**
   *  @brief Callback function for Vulkan to be called by validation layers when needed
*/
static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
    void *pUserData)
{
    std::string messageLevel = "";
    
    if (messageType == 0x00000001)
        messageLevel = "Info";
    if (messageType == 0x00000002)
        messageLevel = "Validation Error";
    if (messageType == 0x00000004)
        messageLevel = "Performance Issue (Not Optimal)";
    std::cerr << "Validation Layer: " << messageLevel << "\n\t" << pCallbackData->pMessage << std::endl;
    return VK_FALSE;
}

/**
   *  @brief This is a proxy function. 
   *  It loads vkCreateDebugUtilsMessengerEXT from memory and then calls it.
   *  This is necessary because this function is an extension function, it is not automatically loaded to memory
*/
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, 
    const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)  
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } 
    else 
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
   *  @brief This is a proxy function. 
   *  It loads vkDestroyDebugUtilsMessengerEXT from memory and then calls it.
   *  This is necessary because this function is an extension function, it is not automatically loaded to memory
*/
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) 
    {
        func(instance, debugMessenger, pAllocator);
    }
}

Device::Device()
{
    CreateInstance();
    SetupDebugMessenger();
}

Device::~Device()
{
    if (m_EnableValidationLayers)
    {
        DestroyDebugUtilsMessengerEXT(m_Instance, m_DebugMessenger, nullptr);
    }

    vkDestroyInstance(m_Instance, nullptr);
}

bool Device::CheckValidationLayerSupport()
{
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    for (const char *layerName : m_ValidationLayers)
    {
        bool layerFound = false;

        for (const auto& layerProperties : availableLayers)
        {
            if (strcmp(layerName, layerProperties.layerName) == 0)
            {
                layerFound = true;
                break;
            }
            
        }
        if (!layerFound)
        {
            return false;
        }
    }    

    return true;
}

std::vector<const char *> Device::GetRequiredGlfwExtensions()
{
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions;
    glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

    if (m_EnableValidationLayers)
    {
        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    return extensions;
}

void Device::CheckRequiredGlfwExtensions()
{
    uint32_t extensionCount = 0;
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> extensions(extensionCount);
    vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());

    std::cout << "Available Extensions:" << std::endl;
    std::unordered_set<std::string> available;
    for (const auto &extension : extensions)
    {
        std::cout << "\t" << extension.extensionName << std::endl;
        available.insert(extension.extensionName);
    }

    std::cout << "Required Extensions:" << std::endl;
    auto requiredExtensions = GetRequiredGlfwExtensions();
    for (const auto& required : requiredExtensions)
    {
        std::cout << "\t" << required << std::endl;
        if (available.find(required) == available.end())
        {
            throw std::runtime_error("Missing required GLFW extension");
        }
    }
}

void Device::SetupDebugMessenger()
{
    if (!m_EnableValidationLayers) return;

    VkDebugUtilsMessengerCreateInfoEXT createInfo{};
    PopulateDebugMessenger(createInfo);

    if (CreateDebugUtilsMessengerEXT(m_Instance, &createInfo, nullptr, &m_DebugMessenger) != VK_SUCCESS)
    {
        throw std::runtime_error("Failed to setup debug messenger!");
    }
}

/**
   *  @brief Sets which messages to show by validation layers
   *
   *  @param createInfo struct to fill
*/
void Device::PopulateDebugMessenger(VkDebugUtilsMessengerCreateInfoEXT& createInfo)
{
    createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
    createInfo.pfnUserCallback = DebugCallback;
}

void Device::CreateInstance()
{
    if (m_EnableValidationLayers && !CheckValidationLayerSupport())
    {
        throw std::runtime_error("Validation layers requested but not avaiable!");
    }

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Gravity";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_0;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{}; // placed outside to ensure that it is not destroyed befoce vkCreateInstance call
    if (m_EnableValidationLayers)
    {
        createInfo.enabledLayerCount = (uint32_t)m_ValidationLayers.size();
        createInfo.ppEnabledLayerNames = m_ValidationLayers.data();

        PopulateDebugMessenger(debugCreateInfo);
        createInfo.pNext = (VkDebugUtilsMessengerCreateFlagsEXT*) &debugCreateInfo;
    }
    else
    {
        createInfo.enabledLayerCount = 0;
        createInfo.pNext = nullptr;
    }

    auto glfwExtensions = GetRequiredGlfwExtensions();
    createInfo.enabledExtensionCount = (uint32_t)glfwExtensions.size();
    createInfo.ppEnabledExtensionNames = glfwExtensions.data();

    if (vkCreateInstance(&createInfo, nullptr, &m_Instance) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create instance!");
    }

    CheckRequiredGlfwExtensions();
}