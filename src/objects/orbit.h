#pragma once

#include "../model.h"
#include "../object.h"

#include <vector>

class Orbit : public Object
{
public:
    Orbit(Device& device, std::vector<Model::Vertex> positions, Transform transform, Properties properties);
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update(std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta) override;
    inline virtual Transform& GetObjectTransform() override { return m_Transform; }
    inline virtual Model* GetObjectModel() override { return  m_Model.get(); }
    inline virtual Properties& GetObjectProperties() override { return m_Properties; };
    inline virtual uint32_t GetObjectID() override { return m_ID; };

    uint32_t m_ID;
    std::unique_ptr<Model> m_Model;
    Transform m_Transform;
    Properties m_Properties;
};