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

    /*
        We need to be able to write our vertex data to memory.
        This property is indicated with VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property.
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is Used to get memory heap that is host coherent. 
        We use this to copy the data into the buffer memory immediately.
    */
    m_Device.CreateBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
        m_StagingBuffer,
        m_StagingBufferMemory
    );
    /*
        When buffer is created It is time to copy the vertex data to the buffer. 
        This is done by mapping the buffer memory into CPU accessible memory with vkMapMemory.
    */
    void *data;
    vkMapMemory(m_Device.GetDevice(), m_StagingBufferMemory, 0, bufferSize, 0, &data);
    memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
    vkUnmapMemory(m_Device.GetDevice(), m_StagingBufferMemory);

    /*
        The vertexBuffer is now allocated from a memory type that is device 
        local, which generally means that we're not able to use vkMapMemory. 
        However, we can copy data from the stagingBuffer to the vertexBuffer. 
        We have to indicate that we intend to do that by specifying the transfer 
        source flag(VK_BUFFER_USAGE_TRANSFER_SRC_BIT) for the stagingBuffer 
        and the transfer destination flag(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        for the vertexBuffer, along with the vertex buffer usage flag.
    */
    m_Device.CreateBuffer(bufferSize, 
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
        m_VertexBuffer,
        m_VertexBufferMemory
    );

    m_Device.CopyBuffer(m_StagingBuffer, m_VertexBuffer, bufferSize);
    vkDestroyBuffer(m_Device.GetDevice(), m_StagingBuffer, nullptr);
    vkFreeMemory(m_Device.GetDevice(), m_StagingBufferMemory, nullptr);
}

/*
    @brief this function call vkCmdBindVertexBuffers which is used to bind vertex buffers to bindings
*/
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
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);

    attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].location = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Vertex, position);

    attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].location = 1;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Vertex, color);
    return attributeDescriptions;
}