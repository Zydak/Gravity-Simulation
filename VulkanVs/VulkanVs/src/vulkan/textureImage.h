#pragma once

#include "device.h"
#include "image.h"
#include <memory>

class TextureImage
{
public:
    TextureImage(Device& device, const std::string& filepath);
    ~TextureImage();

    inline VkImageView GetImageView() { return m_ImageView; }
private:
    void CreateTextureImage(const std::string& filepath);
    void CreateImageView();
    int m_TexWidth, m_TexHeight, m_TexChannels;
    Device& m_Device;

    std::unique_ptr<Image> m_Image;
    VkImageView m_ImageView;
    
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
};