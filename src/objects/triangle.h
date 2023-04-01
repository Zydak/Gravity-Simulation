#pragma once

#include "../model.h"
#include "../object.h"

class Triangle : public Object
{
public:
    Triangle(Device& device, const std::vector<Model::Vertex> &vertices, Transform transform, Properties properties
    );
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update() override;
    virtual Transform GetObjectTransform() override;
    virtual Properties GetObjectProperties() override;

    Model m_Model;
    Transform m_Transform;
    Properties m_Properties;
};