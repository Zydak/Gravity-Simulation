#include "planet.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stbimage/stb_image.h>

#include <iostream>

Planet::Planet(Device& device, const std::string& modelfilepath, Transform transform, Properties properties, const std::string& texturefilepath)
    : m_Model(Model::CreateModelFromFile(device, modelfilepath, texturefilepath)), m_Transform(transform), m_Properties(properties)
{
    static uint32_t currentID = 0;
    m_ID = currentID++;

    OrbitModel::Builder builder;
    if (properties.orbitTraceLenght > 0)
    {
        for (int i = 0; i < properties.orbitTraceLenght; i++)
        {
            builder.vertices.push_back({{0.0f, 0.0f, 0.0f}});
        }
        m_OrbitModel = std::make_unique<OrbitModel>(device, builder);
    }
}

void Planet::Draw(VkCommandBuffer commandBuffer)
{
    m_Model->Bind(commandBuffer);
    m_Model->Draw(commandBuffer);
}

void Planet::DrawOrbit(VkCommandBuffer commandBuffer)
{
    m_OrbitModel->Bind(commandBuffer);
    m_OrbitModel->Draw(commandBuffer);
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

void Planet::OrbitUpdate(VkCommandBuffer commandBuffer)
{
    if (m_Properties.orbitTraceLenght > 0)
    {
        if (m_OrbitPositions.size() >= m_Properties.orbitTraceLenght)
        {
            m_OrbitPositions.erase(m_OrbitPositions.begin());
        }
        m_OrbitPositions.push_back({m_Transform.translation});
        if (!FirstTime && m_OrbitModel->m_Count < m_Properties.orbitTraceLenght)
        {
            m_OrbitModel->m_Count++;
        }
        FirstTime = false;
        
        m_OrbitModel->UpdateVertexBuffer(commandBuffer, m_OrbitModel->GetVertexBuffer(), m_OrbitPositions);
    }
}