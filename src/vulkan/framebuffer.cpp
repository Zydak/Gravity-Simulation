#include "framebuffer.h"

#include <stdexcept>
#include <array>
#include <cassert>
#include "iostream"

void Framebuffer::CreateFramebuffer()
{
    for (auto iter = m_Attachments.begin(); iter != m_Attachments.end(); iter++)
    {
        switch (*iter)
        {
        case FramebufferAttachmentFormat::Presentable:
            CreateImagePresentable();
        break;

        case FramebufferAttachmentFormat::Unorm:
            CreateUnormImage();
        break;

        case FramebufferAttachmentFormat::Depth:
            CreateDepthImage();
        break;
        }
    }

    m_Framebuffers.resize(m_FramebuffersCount);
    for (int i = 0; i < m_FramebuffersCount; i++)
    {
        std::vector<VkImageView> attachments;
        if (!m_PresentableImageViews.empty())
        {
            attachments.push_back(m_PresentableImageViews[i]);
        }
        if (!m_UnormImages.empty())
        {
            attachments.push_back(m_UnormImages[i]->GetImageView());
        }
        if (!m_DepthImages.empty())
        {
            attachments.push_back(m_DepthImages[i]->GetImageView());
        }

        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_RenderPass.GetRenderPass();
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = m_Extent.width;
        framebufferInfo.height = m_Extent.height;
        framebufferInfo.layers = 1;

        if (vkCreateFramebuffer(m_Device.GetDevice(), &framebufferInfo, nullptr, &m_Framebuffers[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create framebuffer!");
        }
    }
}

Framebuffer::Framebuffer(Device& device, VkSwapchainKHR& swapchain, 
        VkExtent2D extent, RenderPass& renderPass, 
        std::vector<FramebufferAttachmentFormat> attachments, uint32_t framebuffersCount
    )
     : m_Device(device), m_Swapchain(swapchain), m_Extent(extent), m_RenderPass(renderPass), m_Attachments(attachments), m_FramebuffersCount(framebuffersCount)
{
    CreateFramebuffer();
}

void Framebuffer::Resize(glm::vec2 size)
{
    m_Extent = {(uint32_t)size.x, (uint32_t)size.y};
    CreateFramebuffer();
}

VkFramebuffer Framebuffer::GetFramebuffer(uint32_t index)
{
    assert(m_FramebuffersCount >= 2 && "More than one framebuffer, please specify index");
    return m_Framebuffers[index];
}

VkFramebuffer Framebuffer::GetFramebuffer()
{
    assert(m_FramebuffersCount < 2 && "More than one framebuffer, please specify index");
    return m_Framebuffers[0];
}

Framebuffer::~Framebuffer()
{
    for (int i = 0; i < m_FramebuffersCount; i++)
    {
        vkDestroyFramebuffer(m_Device.GetDevice(), m_Framebuffers[i], nullptr);
    }
    for (int i = 0; i < m_PresentableImageViews.size(); i++)
    {
        vkDestroyImageView(m_Device.GetDevice(), m_PresentableImageViews[i], nullptr);
    }
}

void Framebuffer::CreateImagePresentable()
{
    m_PresentableImages.resize(m_FramebuffersCount);
    vkGetSwapchainImagesKHR(m_Device.GetDevice(), m_Swapchain, &m_FramebuffersCount, m_PresentableImages.data());

    m_PresentableImageViews.resize(m_FramebuffersCount);

    for (int i = 0; i < m_FramebuffersCount; i++)
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = m_PresentableImages[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = VK_FORMAT_B8G8R8A8_UNORM;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        if (vkCreateImageView(m_Device.GetDevice(), &createInfo, nullptr, &m_PresentableImageViews[i]) != VK_SUCCESS) 
        {
            throw std::runtime_error("failed to create texture image view!");
        }
    }
}

void Framebuffer::CreateUnormImage()
{
    m_UnormImages.resize(m_FramebuffersCount);
    for (int i = 0; i < m_FramebuffersCount; i++)
    {
        VkFormat format = VK_FORMAT_B8G8R8A8_UNORM;
        m_UnormImages[i] = std::make_unique<Image>(m_Device, m_Extent.width, m_Extent.height, format,
            VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT | VK_IMAGE_USAGE_SAMPLED_BIT, VK_IMAGE_ASPECT_COLOR_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
        Image::TransitionImageLayout(m_Device, m_UnormImages[i]->GetImage(), VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
    }
}

void Framebuffer::CreateDepthImage()
{
    m_DepthImages.resize(m_FramebuffersCount);
    for (int i = 0; i < m_FramebuffersCount; i++)
    {
        m_DepthImages[i] = std::make_unique<Image>(m_Device, m_Extent.width, m_Extent.height, VK_FORMAT_D32_SFLOAT
            , VK_IMAGE_TILING_OPTIMAL, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_IMAGE_ASPECT_DEPTH_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
        );
    }
}