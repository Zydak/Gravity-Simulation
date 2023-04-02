#pragma once

#include "../model.h"
#include "../object.h"

class CameraObject : public Object
{
public:
    CameraObject(Device& device, const Transform& transform);
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update() override;
    virtual Transform GetObjectTransform() override { return m_Transform; };
    virtual Properties GetObjectProperties() override { return m_Properties; };

    Transform m_Transform;
    Properties m_Properties{};
};