#pragma once

#include "../models/model.h"
#include "../object.h"
#include "../frameInfo.h"

#include "../descriptors.h"

class Planet : public Object
{
public:
    Planet(uint32_t ID, ObjectInfo objInfo, const std::string& modelfilepath, Transform transform, Properties properties, const std::string& texturefilepath = "");
    virtual void Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer) override;
    virtual void DrawOrbit(VkCommandBuffer commandBuffer) override;
    virtual void Update(std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta, uint32_t substeps) override;
    virtual void OrbitUpdate(VkCommandBuffer commandBuffer) override;
    inline virtual Transform& GetObjectTransform() override { return m_Transform; }
    inline virtual OrbitModel* GetOrbitModel() override { return  m_OrbitModel.get(); }
    inline virtual Model* GetObjectModel() override { return  m_Model.get(); }
    inline virtual Properties& GetObjectProperties() override { return m_Properties; }
    inline virtual uint32_t GetObjectID() override { return m_ID; }
    inline virtual uint32_t GetObjectType() override { return OBJ_TYPE_PLANET; }

    uint32_t m_ID;
    std::unique_ptr<OrbitModel> m_OrbitModel;
    std::unique_ptr<Model> m_Model;
    Transform m_Transform;
    Properties m_Properties;
    std::vector<OrbitModel::Vertex> m_OrbitPositions;
    VkDescriptorSet m_DescriptorSet;
    std::unique_ptr<DescriptorSetLayout> m_SetLayout;

    bool FirstTime = true;
};