#include "planet.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

void Planet::Draw(VkCommandBuffer commandBuffer)
{
    m_Model.Bind(commandBuffer);
    m_Model.Draw(commandBuffer);
}

void Planet::Update()
{
    //m_Transform.rotation.y = glm::mod(m_Transform.rotation.y + 0.01f, glm::two_pi<float>());
    //m_Transform.rotation.x = glm::mod(m_Transform.rotation.x + 0.005f, glm::two_pi<float>());
}

Planet::Planet(Device& device, const std::string& filepath, Transform transform, Properties properties)
    : m_Model(Model::CreateModelFromFile(device, filepath)), m_Transform(transform), m_Properties(properties)
{
    static uint32_t currentID = 0;
    m_ID = currentID++;
}