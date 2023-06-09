#include "object.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stbimage/stb_image.h>

#include <iostream>

Object::Object(uint32_t ID, ObjectInfo objInfo, const std::string& modelfilepath, Transform transform, 
    Properties properties, const std::string& textureFilepath)
    :   m_ID(ID), m_Model(CustomModel::CreateModelFromFile(*objInfo.device, modelfilepath, textureFilepath)),
        m_Transform(transform), m_Properties(properties)
{

    // inclination
    m_Properties.velocity = glm::dvec3(0.0, sin(glm::radians(-m_Properties.inclination)) * m_Properties.velocity.z, 
        cos(glm::radians(-m_Properties.inclination)) * m_Properties.velocity.z
    );

    if (m_Properties.orbitUpdateFrequency <= 0) m_Properties.orbitUpdateFrequency = 1; // can't be zero bcs we're dividing by it later on
    m_Properties.rotationSpeed = glm::radians((m_Properties.rotationSpeed/3600.0)); // convert to radians from degrees
    m_Transform.rotation = glm::radians(m_Transform.rotation);
    m_ObjType = properties.objType;
    m_Radius = properties.radius;
    m_Transform.scale = glm::vec3{1.0f, 1.0f, 1.0f} * m_Radius;
    if (properties.orbitTraceLenght > 0)
    {
        CustomModelPosOnly::Builder builder;
        for (int i = 0; i < properties.orbitTraceLenght; i ++)
        {
            builder.vertices.push_back({m_Transform.translation/SCALE_DOWN});
            m_OrbitPositions.push_back({m_Transform.translation/SCALE_DOWN});
            // we're creating indices array where basically first value is last index to connect everything
            // second is this special value 0xFFFFFFFF and rest is normal i-1
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
        m_OrbitModel = std::make_unique<CustomModelPosOnly>(*objInfo.device, builder);
    }

    auto m_SetLayout = DescriptorSetLayout::Builder(*objInfo.device)
        .AddBinding(0, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
        .Build();
    VkDescriptorImageInfo imageInfo{};
    imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
    imageInfo.imageView = m_Model->GetTextureImage()->GetImageView();
    imageInfo.sampler = objInfo.sampler->GetSampler();
    DescriptorWriter(*m_SetLayout, *objInfo.descriptorPool)
        .WriteImage(0, &imageInfo)
        .Build(m_DescriptorSet);
}

void Object::Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer)
{
    vkCmdBindDescriptorSets(
            commandBuffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            layout,
            1,
            1,
            &m_DescriptorSet,
            0,
            nullptr
    );

    m_Model->Bind(commandBuffer);
    m_Model->Draw(commandBuffer);
}

void Object::DrawOrbit(VkCommandBuffer commandBuffer)
{
    if (m_Properties.orbitTraceLenght > 0)
    {
        m_OrbitModel->Bind(commandBuffer);
        m_OrbitModel->Draw(commandBuffer);
    }
}

void Object::OrbitUpdate(VkCommandBuffer commandBuffer)
{
    if (m_Properties.orbitTraceLenght > 0)
    {
        m_OrbitPositions[m_Count] = {m_Transform.translation/SCALE_DOWN};

        m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetVertexBuffer(), m_Count * sizeof(CustomModelPosOnly::Vertex), sizeof(CustomModelPosOnly::Vertex), (void*)&m_OrbitPositions[m_Count]);
        
        // we're changing index buffer at m_Count offset and adding special value 0xFFFFFFFF because otherwise
        // it would be polygon instead of line
        if (m_Count != m_Properties.orbitTraceLenght-1)
        {
            uint32_t val[2] = {m_Count, 0xFFFFFFFF};
            m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), (m_Count+1) * sizeof(uint32_t), sizeof(uint32_t)*2, (void*)&val);
        }
        // then we're basically reseting the index buffer to it's initial value and repeat the process
        else
        {
            // Proper way
            // This would need new UpdateBuffer function that would update buffer instantly because this one doesn't for some reason
            // uint32_t val[1] = {m_Count};
            // m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), (m_Count+1) * sizeof(uint32_t), sizeof(uint32_t), (void*)&val);
            // val[0] = 0xFFFFFFFF;
            // m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), sizeof(uint32_t), sizeof(uint32_t), (void*)&val);

            m_OrbitModel->UpdateBuffer(commandBuffer, m_OrbitModel->GetIndexBuffer(), 0, m_IndexPositions.size() * sizeof(uint32_t), m_IndexPositions.data());
        }
        
        m_Count = (m_Count+1) % m_Properties.orbitTraceLenght;
    }
}