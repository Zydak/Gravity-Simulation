#pragma once

#include "device.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>

class Model
{
public:
    struct Vertex 
    {
        glm::vec2 position;
        glm::vec3 color;

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();
    };

    Model(Device& device, const std::vector<Vertex> &vertices);
    ~Model();

    Model(const Model&) = delete;
    Model& operator=(const Model&) = delete;

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
    
private:
    void CreateVertexBuffer(const std::vector<Vertex> &vertices);
    void CreateIndexBuffer(const std::vector<uint16_t> &indicies);

    Device &m_Device;
    VkBuffer m_VertexBuffer;
    VkDeviceMemory m_VertexBufferMemory;
    VkBuffer m_StagingBuffer;
    VkDeviceMemory m_StagingBufferMemory;
    uint32_t m_VertexCount;
};