#pragma once

#include "models/orbitModel.h"
#include "models/sphereModel.h"
#include <memory>
#include <unordered_map>

#include <glm/gtc/matrix_transform.hpp>

#define OBJ_TYPE_PLANET 0
#define OBJ_TYPE_STAR 1

struct Transform
{
    glm::dvec3 translation{};
    glm::dvec3 scale{1.0f, 1.0f, 1.0f};
    glm::dvec3 rotation{};
    
    glm::mat4 mat4()
    {
        auto transform = glm::translate(glm::dmat4{1.0f}, translation);

        transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
        transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
        transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.0f});
        transform = glm::scale(transform, scale);
        return transform;
    }
};

struct Properties
{
    glm::dvec3 velocity = {0.0f, 0.0f, 0.0f};
    double mass = 1000.0;
    /*
        @brief static means other object can't affect velocity but it is still applied
    */
    bool isStatic = false;
    uint32_t orbitTraceLenght = 200;
    glm::vec3 rotationSpeed = {0.0f, 0.0f, 0.0f};
    uint32_t objType;
    double radius = 1.0;
};

class Object
{
public:
    Object() = default;
    virtual ~Object() = default;

    virtual Properties& GetObjectProperties() = 0;
    virtual Transform& GetObjectTransform() = 0;
    virtual uint32_t GetObjectID() = 0;
    virtual OrbitModel* GetOrbitModel() = 0;
    virtual SphereModel* GetObjectModel() = 0;
    virtual void Draw(VkPipelineLayout layout, VkCommandBuffer commandBuffer) = 0;
    virtual void DrawOrbit(VkCommandBuffer commandBuffer) = 0;
    virtual void OrbitUpdate(VkCommandBuffer commandBuffer) = 0;
    virtual uint32_t GetObjectType() = 0;

    Transform m_Transform;
    Properties m_Properties;
};