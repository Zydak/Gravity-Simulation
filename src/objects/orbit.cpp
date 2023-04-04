#include "orbit.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <iostream>

void Orbit::Draw(VkCommandBuffer commandBuffer)
{
    m_Model->Bind(commandBuffer);
    m_Model->Draw(commandBuffer);
}

void Orbit::Update(std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta)
{
    
}

Orbit::Orbit(Device& device, std::vector<Model::Vertex> positions, Transform transform, Properties properties)
{
    Model::Builder builder;
    for (int i = 0; i < positions.size(); i++)
    {
        builder.vertices.push_back(Model::Vertex(positions[i]));
    }
    m_Model = std::make_unique<Model>(device, builder);
    static uint32_t currentID = 0;
    m_ID = currentID++;
}