#pragma once

#include "device.h"
#include "buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class Model
{
public:
    struct Vertex 
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 uv;

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        bool operator==(const Vertex& other) const
        {
            return position == other.position && color == other.color && normal == other.normal && uv == other.uv;
        }
    };
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices;

        void LoadModel(const std::string& filepath);
    };

    Model(Device& device, const Model::Builder& builder);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    static Model CreateModelFromFile(Device& device, const std::string& filepath);

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
    
private:
    void CreateVertexBuffer(const std::vector<Vertex> &vertices);
    void CreateIndexBuffer(const std::vector<uint32_t> &indices);

    Device &m_Device;

    std::unique_ptr<Buffer> m_VertexBuffer;
    uint32_t m_VertexCount;

    bool m_HasIndexBuffer = false;
    std::unique_ptr<Buffer> m_IndexBuffer;
    uint32_t m_IndexCount;
};