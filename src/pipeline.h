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

    static PipelineConfigInfo DefaultPipelineConfigInfo(uint32_t width, uint32_t height);
    static PipelineConfigInfo LinesPipelineConfigInfo(uint32_t width, uint32_t height);
    static PipelineConfigInfo SimplePipelineConfigInfo(uint32_t width, uint32_t height);
    
    void Bind(VkCommandBuffer commandBuffer);
    void CreateGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);
    void CreateLinesPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);
    void CreateSimplePipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);
    void CreateSkyboxPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);
private:
    static std::vector<char> ReadFile(const std::string& filepath);

    void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    Device& m_Device;
    VkPipeline m_Pipeline;
    VkShaderModule m_VertexShaderModule;
    VkShaderModule m_FragmentShaderModule;
};