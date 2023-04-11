#include "orbitModel.h"
#include "../utils.h"

#include <cstring>

#define GLM_ENABLE_EXPERIMENTAL

OrbitModel::OrbitModel(Device& device, const OrbitModel::Builder& builder)
    : m_Device(device)
{
    CreateVertexBuffer(builder.vertices);
}
OrbitModel::~OrbitModel()
{

}

void OrbitModel::CreateVertexBuffer(const std::vector<Vertex> &vertices)
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
void OrbitModel::Bind(VkCommandBuffer commandBuffer)
{
    VkBuffer buffers[] = {m_VertexBuffer->GetBuffer()};
    VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
}

void OrbitModel::Draw(VkCommandBuffer commandBuffer)
{
	vkCmdDraw(commandBuffer, m_Count, 1, 0, 0);
}

std::vector<VkVertexInputBindingDescription> OrbitModel::Vertex::GetBindingDescriptions()
{
    std::vector<VkVertexInputBindingDescription> bindingDescription(1);
    bindingDescription[0].binding = 0;
    bindingDescription[0].stride = sizeof(Vertex);
    bindingDescription[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

    return bindingDescription;
}

std::vector<VkVertexInputAttributeDescription> OrbitModel::Vertex::GetAttributeDescriptions()
{
    std::vector<VkVertexInputAttributeDescription> attributeDescriptions{};
    attributeDescriptions.push_back({0, 0, VK_FORMAT_R32G32B32_SFLOAT, offsetof(Vertex, position)});

    return attributeDescriptions;
}

void OrbitModel::UpdateVertexBuffer(VkCommandBuffer cmd, Buffer* buffer, const std::vector<Vertex> &vertices)
{
    vkCmdUpdateBuffer(cmd, buffer->GetBuffer(), 0, sizeof(vertices[0]) * vertices.size(), vertices.data());
}