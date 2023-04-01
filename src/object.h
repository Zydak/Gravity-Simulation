#pragma once

#include "model.h"
#include <memory>

#include <glm/gtc/matrix_transform.hpp>

struct Transform
{
    glm::vec3 translation{};
    glm::vec3 scale{1.0f, 1.0f, 1.0f};
    glm::vec3 rotation{};

    glm::mat4 mat4()
    {
        auto transform = glm::translate(glm::mat4{1.0f}, translation);

        transform = glm::rotate(transform, rotation.y, {0.0f, 1.0f, 0.0f});
        transform = glm::rotate(transform, rotation.x, {1.0f, 0.0f, 0.0f});
        transform = glm::rotate(transform, rotation.z, {0.0f, 0.0f, 1.0f});
        transform = glm::scale(transform, scale);
        return transform;
    }
};

struct Properties
{
    glm::vec3 color;
};

class Object
{
public:
    Object() = default;
    virtual ~Object() = default;

    virtual Properties GetObjectProperties() = 0;
    virtual Transform GetObjectTransform() = 0;
    virtual void Draw(VkCommandBuffer commandBuffer) = 0;
    virtual void Update() = 0;
};

/*
class Object
{
public:
    using id_t = unsigned int;

    static Object CreateGameObject()
    {
        static id_t currentId = 0;
        return Object{currentId++};
    }

    Object(const Object&) = delete;
    Object &operator=(const Object&) = delete;
    Object(Object&&) = default;
    Object &operator=(Object&&) = default;

    inline id_t GetId() { return m_Id; }

    std::shared_ptr<Model> m_Model;
    glm::vec3 m_Color;
    Transform2d m_Transform2d;
private:
    Object(id_t objId) : m_Id(objId) {}

    id_t m_Id;
};*/