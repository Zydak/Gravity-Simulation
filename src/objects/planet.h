#pragma once

#include "../model.h"
#include "../object.h"

class Planet : public Object
{
public:
    Planet(Device& device, const std::string& filepath, Transform transform, Properties properties);
    virtual void Draw(VkCommandBuffer commandBuffer) override;
    virtual void Update(std::unordered_map<int, std::shared_ptr<Object>> gameObjects, float delta) override;
    inline virtual Transform& GetObjectTransform() override { return m_Transform; }
    inline virtual Properties& GetObjectProperties() override { return m_Properties; };
    inline virtual uint32_t GetObjectID() override { return m_ID; };

    uint32_t m_ID;
    Model m_Model;
    Transform m_Transform;
    Properties m_Properties;
};