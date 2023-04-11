#pragma once

#include "../device.h"
#include "../buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class OrbitModel
{
public:
    struct Vertex 
    {
        glm::vec3 position;

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices;
    };

    OrbitModel(Device& device, const OrbitModel::Builder& builder);
    ~OrbitModel();

    OrbitModel(const OrbitModel&) = delete;
    OrbitModel& operator=(const OrbitModel&) = delete;

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
    
    void UpdateVertexBuffer(VkCommandBuffer cmd, Buffer* buffer, const std::vector<Vertex> &vertices);
    inline Buffer* GetVertexBuffer() { return m_VertexBuffer.get(); }
    uint32_t m_Count = 0;
private:
    void CreateVertexBuffer(const std::vector<Vertex> &vertices);
    void CreateIndexBuffer(const std::vector<uint32_t> &indices);

    Device &m_Device;

    std::unique_ptr<Buffer> m_VertexBuffer;
    uint32_t m_VertexCount;
};
