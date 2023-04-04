#include "planet.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>

void Planet::Draw(VkCommandBuffer commandBuffer)
{
    m_Model->Bind(commandBuffer);
    m_Model->Draw(commandBuffer);
}

void Planet::Update(std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta)
{
    for (auto i = gameObjects.begin(); i != gameObjects.end(); i++)
    {
        if (i->first != GetObjectID())
        {
            auto& otherObj = i->second;

            if (m_Properties.isStatic == true)
            {
                continue;
            }
            auto otherObjTranslation = otherObj->GetObjectTransform().translation;
            auto otherObjMass = otherObj->GetObjectProperties().mass;

            auto offset = otherObjTranslation - m_Transform.translation;
            float distanceSquared = glm::dot(offset, offset);

            float force = 15.0 * otherObjMass * m_Properties.mass / distanceSquared;
            glm::vec3 trueForce = force * offset / glm::sqrt(distanceSquared);
            m_Properties.velocity += delta * trueForce / m_Properties.mass;
            m_Transform.translation += delta * m_Properties.velocity;
        }
    }
}

Planet::Planet(Device& device, const std::string& filepath, Transform transform, Properties properties)
    : m_Model(Model::CreateModelFromFile(device, filepath)), m_Transform(transform), m_Properties(properties)
{
    static uint32_t currentID = 0;
    m_ID = currentID++;
}