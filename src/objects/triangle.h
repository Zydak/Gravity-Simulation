#pragma once

#include "../model.h"
#include "../object.h"

class Triangle : public Object
{
public:
    Triangle(Device& device, const std::string& filepath, Transform transform, Properties properties);
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update() override;
    inline virtual Transform GetObjectTransform() override { return m_Transform; }
    inline virtual Properties GetObjectProperties() override { return m_Properties; };
    inline virtual uint32_t GetObjectID() override { return m_ID; };

    uint32_t m_ID;
    Model m_Model;
    Transform m_Transform;
    Properties m_Properties;
};