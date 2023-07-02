#pragma once

#include "device.h"
#include "image.h"
#include "renderPass.h"

#include <vector>
#include <memory>
#include "glm/glm.hpp"

class Framebuffer
{
public:
    Framebuffer(Device& device, VkSwapchainKHR& swapchain, VkExtent2D extent, RenderPass& renderPass, 
        std::vector<FramebufferAttachmentFormat> attachments, uint32_t framebuffersCount = 1
    );
    ~Framebuffer();

    VkImageView GetUnormImageView(int index) { return m_UnormImages[index]->GetImageView(); }

    VkFramebuffer GetFramebuffer(uint32_t index);
    VkFramebuffer GetFramebuffer();
    void Resize(glm::vec2 size);
private:
    void CreateFramebuffer();
    void CreateUnormImage();
    void CreateImagePresentable();
    void CreateDepthImage();

    Device& m_Device;
    VkSwapchainKHR& m_Swapchain;
    std::vector<VkFramebuffer> m_Framebuffers;
    RenderPass& m_RenderPass;

    VkExtent2D m_Extent;
    uint32_t m_FramebuffersCount;
    std::vector<FramebufferAttachmentFormat> m_Attachments;
    std::vector<VkImage> m_PresentableImages;
    std::vector<VkImageView> m_PresentableImageViews;
    std::vector<std::unique_ptr<Image>> m_UnormImages;
    std::vector<std::unique_ptr<Image>> m_DepthImages;
};