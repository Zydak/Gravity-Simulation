#include "triangle.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

void Triangle::Draw(VkCommandBuffer commandBuffer)
{
    m_Model->Bind(commandBuffer);
    m_Model->Draw(commandBuffer);
}

void Triangle::Update()
{
    m_Transform2d.rotation = glm::mod(m_Transform2d.rotation + 0.01f, glm::two_pi<float>());
}

/*
Triangle Triangle::CreateTriangle(Device& device, const std::vector<Model::Vertex> &vertices)
{
    auto m_Model = std::make_shared<Model>(device, vertices);
    triangle.m_Color = {0.1f, 0.8f, 0.1f};
    triangle.m_Transform2d.translation.x = 0.2f;
    triangle.m_Transform2d.scale = {2.0f, 0.5f};
    triangle.m_Transform2d.rotation = 0.25f * glm::two_pi<float>();

    return triangle;
}*/

Triangle::Triangle(Device& device, const std::vector<Model::Vertex> &vertices)
{
    m_Model = std::make_shared<Model>(device, vertices);
    m_Properties.color = {0.1f, 0.8f, 0.1f};
    m_Transform2d.translation.x = 0.2f;
    m_Transform2d.scale = {2.0f, 0.5f};
    m_Transform2d.rotation = 0.25f * glm::two_pi<float>();
}

Transform2d Triangle::GetTransform()
{
    return m_Transform2d;
}

Properties Triangle::GetObjectProperties()
{
    return m_Properties;
}