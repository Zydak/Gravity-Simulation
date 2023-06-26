#pragma once

#include "../vulkan/device.h"
#include "../vulkan/buffer.h"
#include "../vulkan/textureImage.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class CustomModelPosOnly
{
public:
    struct Vertex 
    {
        glm::vec3 position;

        static std::vector<VkVertexInputBindingDescription> GetBindingDescriptions();
        static std::vector<VkVertexInputAttributeDescription> GetAttributeDescriptions();

        bool operator==(const Vertex& other) const
        {
            return position == other.position;
        }
    };
    struct Builder
    {
        std::vector<Vertex> vertices{};
        std::vector<uint32_t> indices;

        void LoadModel(const std::string& modelFilepath);
    };

    CustomModelPosOnly(Device& device, const CustomModelPosOnly::Builder& builder);
    ~CustomModelPosOnly();

    CustomModelPosOnly(const CustomModelPosOnly&) = delete;
    CustomModelPosOnly& operator=(const CustomModelPosOnly&) = delete;

    inline TextureImage* GetTextureImage() { return m_TextureImage.get(); }

    static std::unique_ptr<CustomModelPosOnly> CreateModelFromFile(Device& device, const std::string& modelFilepath);

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
    
    void UpdateBuffer(VkCommandBuffer cmd, Buffer* buffer, VkDeviceSize offset, uint32_t size, const void* data);
    inline Buffer* GetVertexBuffer() { return m_VertexBuffer.get(); }
    inline Buffer* GetIndexBuffer() { return m_IndexBuffer.get(); }
    
private:
    void CreateVertexBuffer(const std::vector<Vertex> &vertices);
    void CreateIndexBuffer(const std::vector<uint32_t> &indices);

    Device &m_Device;

    std::unique_ptr<Buffer> m_VertexBuffer;
    uint32_t m_VertexCount;

    bool m_HasIndexBuffer = false;
    std::unique_ptr<Buffer> m_IndexBuffer;
    uint32_t m_IndexCount;

    std::unique_ptr<TextureImage> m_TextureImage;
};