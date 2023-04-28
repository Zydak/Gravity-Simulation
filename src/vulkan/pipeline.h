#pragma once

#include <string>
#include <vector>

#include "device.h"

struct PipelineConfigInfo 
{
    VkViewport viewport;
    VkRect2D scissor;
    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
    VkPipelineRasterizationStateCreateInfo rasterizationInfo;
    VkPipelineMultisampleStateCreateInfo multisampleInfo;
    VkPipelineColorBlendAttachmentState colorBlendAttachment;
    VkPipelineColorBlendStateCreateInfo colorBlendInfo;
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
    VkPipelineLayout pipelineLayout = nullptr;
    VkRenderPass renderPass = nullptr;
    uint32_t subpass = 0;
};

class Pipeline
{
public:
    Pipeline(Device& device);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    
    void Bind(VkCommandBuffer commandBuffer);

    static PipelineConfigInfo CreatePipelineConfigInfo(uint32_t width, uint32_t height,
        VkPrimitiveTopology topology, VkCullModeFlags cullMode);
    void CreatePipeline(const std::string& vertexPath, const std::string& fragmentPath, 
        const PipelineConfigInfo& configInfo,
        std::vector<VkVertexInputBindingDescription> bindingDesc,
        std::vector<VkVertexInputAttributeDescription> attributeDesc
    );
    static void CreatePipelineLayout(Device& device, std::vector<VkDescriptorSetLayout>& 
        descriptorSetsLayouts, VkPushConstantRange& pushConstants, 
        VkPipelineLayout& pipelineLayout
    );

private:
    static std::vector<char> ReadFile(const std::string& filepath);

    void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    Device& m_Device;
    VkPipeline m_Pipeline;
};