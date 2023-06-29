#pragma once
#include "device.h"

class RenderPass
{
public:
    RenderPass(Device& device, VkFormat imageFormat, VkFormat depthFormat);
    ~RenderPass();

    inline VkRenderPass& GetRenderPass() { return m_RenderPass; }

private:
    Device& m_Device;
    VkRenderPass m_RenderPass;
};