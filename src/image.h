#pragma once

#include "device.h"

class Image
{
public:
    Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties);
    ~Image();
    void TransitionImageLayout(VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);

    inline VkImage GetImage() { return m_Image; }
    inline VkDeviceMemory GetImageMemory() { return m_ImageMemory; }

private:
    Device& m_Device;

    VkImage m_Image;
    VkDeviceMemory m_ImageMemory;
};