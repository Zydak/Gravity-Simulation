#pragma once

#include "../model.h"
#include "../object.h"

class Triangle : public Object
{
public:
    Triangle(Device& device, const std::vector<Model::Vertex> &vertices, Transform2d transform, Properties properties
    );
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update() override;
    virtual Transform2d GetObjectTransform() override;
    virtual Properties GetObjectProperties() override;

    Model m_Model;
    Transform2d m_Transform2d;
    Properties m_Properties;
};