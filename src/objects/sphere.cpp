#include "sphere.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

#include <stbimage/stb_image.h>

#include <iostream>

Sphere::Sphere(uint32_t ID, ObjectInfo objInfo, const std::string& modelfilepath, Transform transform, 
    Properties properties, 
    const std::string& texturefilepath)
    :   m_ID(ID), m_Model(SphereModel::CreateModelFromFile(*objInfo.device, modelfilepath, texturefilepath)),
        m_Transform(transform), m_Properties(properties)
{
    m_ObjType = properties.objType;

    //m_Radius = std::cbrt(properties.mass);
    m_Radius = 10.0f;
    m_Transform.scale = glm::vec3{1.0f, 1.0f, 1.0f} * m_Radius;
    if (properties.orbitTraceLenght > 0)
    {
        OrbitModel::Builder builder;
        builder.vertices.resize(properties.orbitTraceLenght);
        m_OrbitModel = std::make_unique<OrbitModel>(*objInfo.device, builder);
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

void Sphere::Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer)
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

void Sphere::DrawOrbit(VkCommandBuffer commandBuffer)
{
    m_OrbitModel->Bind(commandBuffer);
    m_OrbitModel->Draw(commandBuffer);
}

void Sphere::OrbitUpdate(VkCommandBuffer commandBuffer)
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