#pragma once

#include "device.h"

class TextureImage
{
public:
    TextureImage(Device& device, const std::string& filepath);
    ~TextureImage();

    inline VkImageView GetImageView() { return m_ImageView; }
private:
    void CreateTextureImage(const std::string& filepath);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    void CreateImageView();
    int m_TexWidth, m_TexHeight, m_TexChannels;
    Device& m_Device;

    VkImage m_Image;
    VkDeviceMemory m_ImageMemory;
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
    VkImageView m_ImageView;
};