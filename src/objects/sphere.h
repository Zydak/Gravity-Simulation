#pragma once

#include "../models/sphereModel.h"
#include "../object.h"
#include "../frameInfo.h"

#include "../vulkan/descriptors.h"

class Sphere : public Object
{
public:
    Sphere(uint32_t ID, ObjectInfo objInfo, const std::string& modelfilepath, Transform transform, 
        Properties properties
    );
    virtual void Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer) override;
    virtual void DrawOrbit(VkCommandBuffer commandBuffer) override;
    virtual void OrbitUpdate(VkCommandBuffer commandBuffer) override;
    virtual void ChangeOffset(glm::dvec3 offset) override;
    inline virtual Transform& GetObjectTransform() override { return m_Transform; }
    inline virtual OrbitModel* GetOrbitModel() override { return  m_OrbitModel.get(); }
    inline virtual SphereModel* GetObjectModel() override { return  m_Model.get(); }
    inline virtual Properties& GetObjectProperties() override { return m_Properties; }
    inline virtual uint32_t GetObjectID() override { return m_ID; }
    inline virtual uint32_t GetObjectType() override { return m_ObjType; }

    int m_ObjType;
    uint32_t m_ID;
    std::unique_ptr<OrbitModel> m_OrbitModel;
    std::unique_ptr<SphereModel> m_Model;
    Transform m_Transform;
    Properties m_Properties;
    std::vector<OrbitModel::Vertex> m_OrbitPositions;
    VkDescriptorSet m_DescriptorSet;
    std::unique_ptr<DescriptorSetLayout> m_SetLayout;
    glm::dvec3 m_Offset{0.0};

    float m_Radius;
    bool FirstTime = true;
};