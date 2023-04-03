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
    //m_Transform.rotation.y = glm::mod(m_Transform.rotation.y + 0.01f, glm::two_pi<float>());
    //m_Transform.rotation.x = glm::mod(m_Transform.rotation.x + 0.005f, glm::two_pi<float>());
}

Triangle::Triangle(Device& device, const std::string& filepath, Transform transform, Properties properties)
    : m_Model(Model::CreateModelFromFile(device, filepath)), m_Transform(transform), m_Properties(properties)
{
     
}

Transform Triangle::GetObjectTransform()
{
    return m_Transform;
}

Properties Triangle::GetObjectProperties()
{
    return m_Properties;
}