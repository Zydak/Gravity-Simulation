#pragma once

#include "../vulkan/device.h"
#include "../vulkan/buffer.h"
#include "../vulkan/textureImage.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include <vector>
#include <memory>

class SphereModel
{
public:
    struct Vertex 
    {
        glm::vec3 position;
        glm::vec3 color;
        glm::vec3 normal;
        glm::vec2 texCoord;
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

        void LoadModel(const std::string& modelFilepath);
    };

    SphereModel(Device& device, const SphereModel::Builder& builder, const std::string& textureFilepath);
    ~SphereModel();

    SphereModel(const SphereModel&) = delete;
    SphereModel& operator=(const SphereModel&) = delete;

    inline TextureImage* GetTextureImage() { return m_TextureImage.get(); }

    static std::unique_ptr<SphereModel> CreateModelFromFile(Device& device, const std::string& modelFilepath, const std::string& textureFilepath = "");

    void Bind(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
    
    void UpdateVertexBuffer(VkCommandBuffer cmd, Buffer* buffer, const std::vector<Vertex> &vertices);
    inline Buffer* GetVertexBuffer() { return m_VertexBuffer.get(); }
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