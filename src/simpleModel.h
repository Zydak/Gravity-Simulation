#pragma once

#include "device.h"
#include "buffer.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class SimpleModel
{
public:
    struct Vertex 
    {
        glm::vec2 pos;
        glm::vec3 color;
        glm::vec2 texCoord;

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions() {
            std::vector<VkVertexInputBindingDescription> bindingDescription(1);
            bindingDescription[0].binding = 0;
            bindingDescription[0].stride = sizeof(Vertex);
            bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

            return bindingDescription;
        }

        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions() 
        {
            std::vector<VkVertexInputAttributeDescription> attributeDescriptions(3);

            attributeDescriptions[0].binding = 0;
            attributeDescriptions[0].location = 0;
            attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[0].offset = offsetof(Vertex, pos);

            attributeDescriptions[1].binding = 0;
            attributeDescriptions[1].location = 1;
            attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
            attributeDescriptions[1].offset = offsetof(Vertex, color);

            attributeDescriptions[2].binding = 0;
            attributeDescriptions[2].location = 2;
            attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
            attributeDescriptions[2].offset = offsetof(Vertex, texCoord);

            return attributeDescriptions;
        }
    };

    SimpleModel(Device& device, const std::vector<Vertex>& vertices, std::vector<uint32_t> indices);
    ~SimpleModel();

    SimpleModel(const SimpleModel&) = delete;
    SimpleModel& operator=(const SimpleModel&) = delete;

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