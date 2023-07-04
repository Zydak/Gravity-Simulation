#pragma once

#include "device.h"
#include "image.h"
#include "sampler.h"
#include <memory>

class TextureImage
{
public:
    TextureImage(Device& device, const std::string& filepath, bool descriptor = false);
    ~TextureImage();

    inline VkImageView GetImageView() { return m_Image->GetImageView(); }
    inline Image* GetImage() { return m_Image.get(); }
    inline uint32_t GetWidth() { return m_TexWidth; }
    inline uint32_t GetHeight() { return m_TexHeight; }
private:
    void CreateTextureImage(const std::string& filepath);
    int m_TexWidth, m_TexHeight, m_TexChannels;
    Device& m_Device;

    std::unique_ptr<Image> m_Image;
    
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
};