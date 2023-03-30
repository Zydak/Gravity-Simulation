#pragma once

#include "../model.h"
#include "../object.h"

class Triangle : Object
{
public:
    Triangle(const Triangle&) = delete;
    Triangle &operator=(const Object&) = delete;
    Triangle(Triangle&&) = default;
    Triangle &operator=(Triangle&&) = default;

    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update() override;
    static Triangle CreateTriangle(Device& device, const std::vector<Model::Vertex> &vertices);
    virtual Transform2d GetTransform() override;
    virtual Properties GetObjectProperties() override;

    std::shared_ptr<Model> m_Model;
    Transform2d m_Transform2d;
    Properties m_Properties;

    Triangle(Device& device, const std::vector<Model::Vertex> &vertices);
private:
};