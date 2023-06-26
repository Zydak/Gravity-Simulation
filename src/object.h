#pragma once

#include "models/customModel.h"
#include "models/customModelPosOnly.h"
#include "object.h"

#include "vulkan/descriptors.h"
#include "vulkan/sampler.h"

#include <glm/gtc/matrix_transform.hpp>

#define OBJ_TYPE_PLANET 0
#define OBJ_TYPE_STAR 1

#define SCALE_DOWN 1000000000.0    // scaling down every position for rendering because skybox breaks you will see when you scroll back really far away

struct ObjectInfo
{
    Device* device;
    Sampler* sampler;
    DescriptorPool* descriptorPool;
};

struct Transform
{
    glm::dvec3 translation{0.0, 0.0, 0.0};
    glm::dvec3 scale{1.0, 1.0, 1.0};
    glm::dvec3 rotation{};
    
    glm::mat4 mat4()
    {
        auto transform = glm::translate(glm::dmat4{1.0f}, (translation)/SCALE_DOWN);

        transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
        transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
        transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.0f});
        transform = glm::scale(transform, scale/SCALE_DOWN);
        return transform;
    }
};

struct Properties
{
    std::string label;
    float orbitUpdateFrequency;
    glm::dvec3 velocity = {0.0f, 0.0f, 0.0f};
    double mass = 1000.0;
    uint32_t orbitTraceLenght = 200;
    glm::dvec3 rotationSpeed = {0.0f, 0.0f, 0.0f};
    uint32_t objType;
    double radius = 1.0;
    glm::vec3 color;
    double inclination;
};

class Object
{
public:
    Object(uint32_t ID, ObjectInfo objInfo, const std::string& modelfilepath, Transform transform, 
        Properties properties, const std::string& textureFilepath = ""
    );
    void Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer);
    void DrawOrbit(VkCommandBuffer commandBuffer);
    void OrbitUpdate(VkCommandBuffer commandBuffer);
    inline Transform& GetObjectTransform() { return m_Transform; }
    inline Properties& GetObjectProperties() { return m_Properties; }
    inline uint32_t GetObjectID() { return m_ID; }
    inline uint32_t GetObjectType() { return m_ObjType; }
    inline glm::vec3 GetObjectColor() { return m_Properties.color; }
    inline std::string GetObjectLabel() { return m_Properties.label; }
    inline uint32_t GetObjectOrbitUpdateFreq() { return m_Properties.orbitUpdateFrequency; }

    int m_ObjType;
    uint32_t m_ID;
    std::unique_ptr<CustomModelPosOnly> m_OrbitModel;
    std::unique_ptr<CustomModel> m_Model;
    Transform m_Transform;
    Properties m_Properties;
    VkDescriptorSet m_DescriptorSet;
    std::unique_ptr<DescriptorSetLayout> m_SetLayout;
    uint32_t m_Count = 0;

    std::vector<CustomModelPosOnly::Vertex> m_OrbitPositions;
    std::vector<uint32_t> m_IndexPositions;

    float m_Radius;
};