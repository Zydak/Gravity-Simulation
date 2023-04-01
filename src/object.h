#pragma once

#include "model.h"
#include <memory>

struct Transform2d
{
    glm::vec2 translation;
    glm::vec2 scale{1.0f, 1.0f};
    float rotation;

    glm::mat2 mat2() 
    { 
        const float s = glm::sin(rotation);
        const float c = glm::cos(rotation);
        glm::mat2 rotMatrix{{c, s}, {-s, c}};

        glm::mat2 scaleMat{{scale.x, 0.0f,}, {0.0f, scale.y}};
        return rotMatrix * scaleMat; 
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
    virtual Transform2d GetObjectTransform() = 0;
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