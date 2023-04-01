#include "triangle.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

void Triangle::Draw(VkCommandBuffer commandBuffer)
{
    m_Model.Bind(commandBuffer);
    m_Model.Draw(commandBuffer);
}

void Triangle::Update()
{
    m_Transform2d.rotation = glm::mod(m_Transform2d.rotation + 0.01f, glm::two_pi<float>());
}

Triangle::Triangle(Device& device, const std::vector<Model::Vertex> &vertices, Transform2d transform, Properties properties)
    : m_Model(device, vertices), m_Transform2d(transform), m_Properties(properties)
{
    
}

Transform2d Triangle::GetObjectTransform()
{
    return m_Transform2d;
}

Properties Triangle::GetObjectProperties()
{
    return m_Properties;
}