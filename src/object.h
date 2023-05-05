#pragma once

#include "models/orbitModel.h"
#include "models/sphereModel.h"
#include <memory>
#include <unordered_map>
#include <iostream>

#include <glm/gtc/matrix_transform.hpp>

#define OBJ_TYPE_PLANET 0
#define OBJ_TYPE_STAR 1

//10000000000000000000
#define SCALE_DOWN 10000000.0    // scaling down every position for rendering because using double in shaders 
                                  // is not possible and float breaks on big distances. It doesn't work on really big scales anyway tho

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
    glm::dvec3 velocity = {0.0f, 0.0f, 0.0f};
    double mass = 1000.0;
    uint32_t orbitTraceLenght = 200;
    glm::vec3 rotationSpeed = {0.0f, 0.0f, 0.0f};
    uint32_t objType;
    double radius = 1.0;
    glm::vec3 color;
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
    virtual glm::vec3 GetObjectColor() = 0;

    Transform m_Transform;
    Properties m_Properties;
};