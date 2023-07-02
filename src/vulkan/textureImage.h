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

    inline VkImageView GetImageView() { return m_ImageView; }
    inline Image* GetImage() { return m_Image.get(); }
    VkDescriptorSet GetImageDescriptor();
    inline uint32_t GetWidth() { return m_TexWidth; }
    inline uint32_t GetHeight() { return m_TexHeight; }
private:
    void CreateTextureImage(const std::string& filepath);
    void CreateImageView();
    int m_TexWidth, m_TexHeight, m_TexChannels;
    Device& m_Device;
    Sampler m_Sampler;
    VkDescriptorSet m_Descriptor;

    std::unique_ptr<Image> m_Image;
    VkImageView m_ImageView;
    
    VkBuffer m_Buffer;
    VkDeviceMemory m_BufferMemory;
};