#include "sphere.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stbimage/stb_image.h>

#include <iostream>

Sphere::Sphere(uint32_t ID, ObjectInfo objInfo, const std::string& modelfilepath, Transform transform, 
    Properties properties)
    :   m_ID(ID), m_Model(SphereModel::CreateModelFromFile(*objInfo.device, modelfilepath)),
        m_Transform(transform), m_Properties(properties)
{
    m_ObjType = properties.objType;
    m_Radius = properties.radius;
    m_Transform.scale = glm::vec3{1.0f, 1.0f, 1.0f} * m_Radius;
    if (properties.orbitTraceLenght > 0)
    {
        OrbitModel::Builder builder;
        for (int i = 0; i < properties.orbitTraceLenght; i ++)
        {
            builder.vertices.push_back({m_Transform.translation/SCALE_DOWN});
            m_OrbitPositions.push_back({m_Transform.translation/SCALE_DOWN});
            if (i == 0)
            {
                builder.indices.push_back(properties.orbitTraceLenght-1);
                m_IndexPositions.push_back(properties.orbitTraceLenght-1);
            }
            else if (i == 1)
            {
                m_IndexPositions.push_back(0xFFFFFFFF);
                builder.indices.push_back(0xFFFFFFFF);
            }
            else
            {
                builder.indices.push_back(i-1);
                m_IndexPositions.push_back(i-1);
            }
        }
        builder.indices.push_back(properties.orbitTraceLenght-1);
        m_IndexPositions.push_back(properties.orbitTraceLenght-1);
        m_OrbitModel = std::make_unique<OrbitModel>(*objInfo.device, builder);
    }
}

void Sphere::Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer)
{
    m_Model->Bind(commandBuffer);
    m_Model->Draw(commandBuffer);
}

void Sphere::DrawOrbit(VkCommandBuffer commandBuffer)
{
    if (m_Properties.orbitTraceLenght > 0)
    {
        m_OrbitModel->Bind(commandBuffer);
        m_OrbitModel->Draw(commandBuffer);
    }
}

void Sphere::OrbitUpdate(VkCommandBuffer commandBuffer)
{
    if (m_Properties.orbitTraceLenght > 0)
    {
        m_OrbitPositions[m_Count] = {m_Transform.translation/SCALE_DOWN};

        m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetVertexBuffer(), m_Count * sizeof(OrbitModel::Vertex), sizeof(OrbitModel::Vertex), (void*)&m_OrbitPositions[m_Count]);
        
        if (m_Count != m_Properties.orbitTraceLenght-1)
        {
            uint32_t val[2] = {m_Count, 0xFFFFFFFF};
            m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), (m_Count+1) * sizeof(uint32_t), sizeof(uint32_t)*2, (void*)&val);
        }
        else
        {
            // Proper way
            // This would need new UpdateBuffer function that would update buffer instantly
            // uint32_t val[1] = {m_Count};
            // m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), (m_Count+1) * sizeof(uint32_t), sizeof(uint32_t), (void*)&val);
            // val[0] = 0xFFFFFFFF;
            // m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), sizeof(uint32_t), sizeof(uint32_t), (void*)&val);

            m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), 0, m_IndexPositions.size() * sizeof(uint32_t), m_IndexPositions.data());
        }
        
        m_Count = (m_Count+1) % m_Properties.orbitTraceLenght;
    }
}