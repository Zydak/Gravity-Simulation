#include "simpleModel.h"
#include "utils.h"

#include <cassert>
#include <cstring>
#include <unordered_map>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

SimpleModel::SimpleModel(Device& device, const std::vector<Vertex>& vertices, std::vector<uint32_t> indices)
    : m_Device(device)
{
    CreateVertexBuffer(vertices);
    CreateIndexBuffer(indices);
}
SimpleModel::~SimpleModel()
{

}

void SimpleModel::CreateVertexBuffer(const std::vector<Vertex> &vertices)
{
    m_VertexCount = static_cast<uint32_t>(vertices.size());
    //assert(m_VertexCount >= 3 && "Vertex count me be at least 3");
    VkDeviceSize bufferSize = sizeof(vertices[0]) * m_VertexCount;
    uint32_t vertexSize = sizeof(vertices[0]);

    /*
        We need to be able to write our vertex data to memory.
        This property is indicated with VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property.
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is Used to get memory heap that is host coherent. 
        We use this to copy the data into the buffer memory immediately.
    */
    Buffer stagingBuffer(m_Device, 
        vertexSize, 
        m_VertexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    /*
        When buffer is created It is time to copy the vertex data to the buffer. 
        This is done by mapping the buffer memory into CPU accessible memory with vkMapMemory.
    */
    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)vertices.data());

    /*
        The vertexBuffer is now allocated from a memory type that is device 
        local, which generally means that we're not able to use vkMapMemory. 
        However, we can copy data from the stagingBuffer to the vertexBuffer. 
        We have to indicate that we intend to do that by specifying the transfer 
        source flag(VK_BUFFER_USAGE_TRANSFER_SRC_BIT) for the stagingBuffer 
        and the transfer destination flag(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        for the vertexBuffer, along with the vertex buffer usage flag.
    */

    m_VertexBuffer = std::make_unique<Buffer>(
        m_Device,
        vertexSize,
        m_VertexCount,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_VertexBuffer->GetBuffer(), bufferSize);
}

/*
    @brief this function call vkCmdBindVertexBuffers which is used to bind vertex buffers to bindings
*/
void SimpleModel::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_VertexBuffer->GetBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);

    if (m_HasIndexBuffer)
    {
        vkCmdBindIndexBuffer(commandBuffer, m_IndexBuffer->GetBuffer(), 0, VK_INDEX_TYPE_UINT32);
    }
}

void SimpleModel::Draw(VkCommandBuffer commandBuffer)
{
    if (m_HasIndexBuffer)
    {
        vkCmdDrawIndexed(commandBuffer, m_IndexCount, 1, 0, 0, 0);
    }
    else
    {
        vkCmdDraw(commandBuffer, m_VertexCount, 1, 0, 0);
    }
}


void SimpleModel::CreateIndexBuffer(const std::vector<uint32_t> &indices)
{
    m_IndexCount = static_cast<uint32_t>(indices.size());
    m_HasIndexBuffer = m_IndexCount > 0;
    if (!m_HasIndexBuffer)
    {
        return;
    }

    VkDeviceSize bufferSize = sizeof(indices[0]) * m_IndexCount;
    uint32_t indexSize = sizeof(indices[0]);

    /*
        We need to be able to write our index data to memory.
        This property is indicated with VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT with VK_MEMORY_PROPERTY_HOST_COHERENT_BIT property.
        VK_MEMORY_PROPERTY_HOST_COHERENT_BIT is Used to get memory heap that is host coherent. 
        We use this to copy the data into the buffer memory immediately.
    */
    Buffer stagingBuffer(
        m_Device,
        indexSize,
        m_IndexCount,
        VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
        VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
    );

    /*
        When buffer is created It is time to copy the index data to the buffer. 
        This is done by mapping the buffer memory into CPU accessible memory with vkMapMemory.
    */
    stagingBuffer.Map();
    stagingBuffer.WriteToBuffer((void*)indices.data());

    /*
        The IndexBuffer is now allocated from a memory type that is device 
        local, which generally means that we're not able to use vkMapMemory. 
        However, we can copy data from the stagingBuffer to the IndexBuffer. 
        We have to indicate that we intend to do that by specifying the transfer 
        source flag(VK_BUFFER_USAGE_TRANSFER_SRC_BIT) for the stagingBuffer 
        and the transfer destination flag(VK_BUFFER_USAGE_TRANSFER_DST_BIT)
        for the IndexBuffer, along with the IndexBuffer usage flag.
    */
    m_IndexBuffer = std::make_unique<Buffer>(
        m_Device,
        indexSize,
        m_IndexCount,
        VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT, 
        VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
    );

    m_Device.CopyBuffer(stagingBuffer.GetBuffer(), m_IndexBuffer->GetBuffer(), bufferSize);
}
