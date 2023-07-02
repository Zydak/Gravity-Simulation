#pragma once

#include "device.h"

class Image
{
public:
    Image(Device& device, uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, 
        VkImageUsageFlags usage, VkImageAspectFlagBits imageAspect, VkMemoryPropertyFlags properties
    );
    ~Image();
    static void TransitionImageLayout(Device& device, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout);
    static void TransitionImageLayout(Device& device, VkImage image, VkImageLayout oldLayout, VkImageLayout newLayout, VkImageSubresourceRange subresourceRange);
    void CopyBufferToImage(VkBuffer buffer, uint32_t width, uint32_t height);
    void WriteDataToImage(void* data);

    inline VkImage GetImage() { return m_Image; }
    inline VkImageView GetImageView() { return m_ImageView; }
    inline VkDeviceMemory GetImageMemory() { return m_ImageMemory; }

private:
    Device& m_Device;

    uint32_t m_Width, m_Height;

    VkImage m_Image;
    VkImageView m_ImageView;
    VkDeviceMemory m_ImageMemory;
};