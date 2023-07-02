#pragma once

#include "device.h"
#include "image.h"
#include "renderPass.h"

#include <vector>
#include <memory>

enum FramebufferAttachmentFormat
{
    Depth,
    Unorm,
    Presentable
};

class Framebuffer
{
public:
    Framebuffer(Device& device, VkSwapchainKHR& swapchain, VkExtent2D extent, RenderPass& renderPass, 
        std::vector<FramebufferAttachmentFormat> attachments, uint32_t framebuffersCount = 1
    );
    ~Framebuffer();

    VkFramebuffer GetFramebuffer(uint32_t index);
    VkFramebuffer GetFramebuffer();
private:
    void CreateUnormImage();
    void CreateImagePresentable();
    void CreateDepthImage();

    Device& m_Device;
    VkSwapchainKHR& m_Swapchain;
    std::vector<VkFramebuffer> m_Framebuffers;

    VkExtent2D m_Extent;
    uint32_t m_FramebuffersCount;
    std::vector<VkImage> m_PresentableImages;
    std::vector<VkImageView> m_PresentableImageViews;
    std::vector<std::unique_ptr<Image>> m_UnormImages;
    std::vector<std::unique_ptr<Image>> m_DepthImages;
};