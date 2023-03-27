#include "model.h"

#include <cassert>
#include <cstring>

Model::Model(Device& device, const std::vector<Vertex> &vertices)
    : m_Device(device)
{
    CreateVertexBuffer(vertices);
}
Model::~Model()
{
    vkDestroyBuffer(m_Device.GetDevice(), m_VertexBuffer, nullptr);
    vkFreeMemory(m_Device.GetDevice(), m_VertexBufferMemory, nullptr);
}

void Model::CreateVertexBuffer(const std::vector<Vertex> &vertices)
{
    m_VertexCount = static_cast<uint32_t>(vertices.size());
    assert(m_VertexCount >= 3 && "Vertex count me be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;

    m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_StagingBuffer,
        m_StagingBufferMemory
        );
    void *data;
    vkMapMemory(m_Device.GetDevice(), m_StagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_Device.GetDevice(), m_StagingBufferMemory);

    m_Device.CreateBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_DST_BIT 
        | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VertexBuffer, m_VertexBufferMemory);

    m_Device.CopyBuffer(m_StagingBuffer, m_VertexBuffer, bufferSize);
    vkDestroyBuffer(m_Device.GetDevice(), m_StagingBuffer, nullptr);
    vkFreeMemory(m_Device.GetDevice(), m_StagingBufferMemory, nullptr);
}


void Model::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_VertexBuffer};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void Model::Draw(VkCommandBuffer commandBuffer)
{
    vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> Model::Vertex::GetBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescription(1);
    bindingDescription[0].binding = 0;
    bindingDescription[0].stride = sizeof(Vertex);
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> Model::Vertex::GetAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, pos);

    return attributeDescriptions;
}