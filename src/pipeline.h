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
    Pipeline(Device& device, 
        const std::string& vertexPath, 
        const std::string& fragmentPath, 
        const PipelineConfigInfo& configInfo);
    ~Pipeline();

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;

    static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);
    static PipelineConfigInfo LinesPipelineConfigInfo(uint32_t width, uint32_t height);
    void Bind(VkCommandBuffer commandBuffer);
private:
    static std::vector<char> ReadFile(const std::string& filepath);

    void CreateGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);

    void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    Device& m_Device;
    VkPipeline m_Pipeline;
    VkShaderModule m_VertexShaderModule;
    VkShaderModule m_FragmentShaderModule;
};