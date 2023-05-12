#include "textureImage.h"

#define STB_IMAGE_IMPLEMENTATION
#include <stbimage/stb_image.h>

#include "stdexcept"

TextureImage::TextureImage(Device& device, const std::string& filepath)
    : m_Device(device)
{
    CreateTextureImage(filepath);
    CreateImageView();
}

TextureImage::~TextureImage()
{
    vkDeviceWaitIdle(m_Device.GetDevice());
    vkDestroyImageView(m_Device.GetDevice(), m_ImageView, nullptr);
}

void TextureImage::CreateTextureImage(const std::string& filepath) 
{
    stbi_uc* pixels = stbi_load(filepath.c_str(), &m_TexWidth, &m_TexHeight, &m_TexChannels, STBI_rgb_alpha);
    VkDeviceSize imageSize = m_TexWidth * m_TexHeight * 4;

    if (!pixels) 
    {
        throw std::runtime_error("failed to load texture image!");
    }
    
    m_Device.CreateBuffer(imageSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, 
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, m_Buffer, m_BufferMemory);

    void* data;
    vkMapMemory(m_Device.GetDevice(), m_BufferMemory, 0, imageSize, 0, &data);
    memcpy(data, pixels, static_cast<size_t>(imageSize));
    vkUnmapMemory(m_Device.GetDevice(), m_BufferMemory);

    stbi_image_free(pixels);

    m_Image = std::make_unique<Image>(m_Device, m_TexWidth, m_TexHeight, 
        VK_FORMAT_R8G8B8A8_UNORM, VK_IMAGE_TILING_OPTIMAL, 
        VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    Image::TransitionImageLayout(m_Device, m_Image->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
    m_Image->CopyBufferToImage(m_Buffer, static_cast<uint32_t>(m_TexWidth), static_cast<uint32_t>(m_TexHeight));
    Image::TransitionImageLayout(m_Device, m_Image->GetImage(), VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    
    vkDestroyBuffer(m_Device.GetDevice(), m_Buffer, nullptr);
    vkFreeMemory(m_Device.GetDevice(), m_BufferMemory, nullptr);
}

void TextureImage::CreateImageView()
{
    VkImageViewCreateInfo viewInfo{};
    viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    viewInfo.image = m_Image->GetImage();
    viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
    viewInfo.format = VK_FORMAT_R8G8B8A8_UNORM;
    viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    viewInfo.subresourceRange.baseMipLevel = 0;
    viewInfo.subresourceRange.levelCount = 1;
    viewInfo.subresourceRange.baseArrayLayer = 0;
    viewInfo.subresourceRange.layerCount = 1;
    if (vkCreateImageView(m_Device.GetDevice(), &viewInfo, nullptr, &m_ImageView) != VK_SUCCESS) 
    {
        throw std::runtime_error("failed to create texture image view!");
    }
}