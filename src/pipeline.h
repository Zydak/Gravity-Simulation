#pragma once

#include <string>
#include <vector>

#include "device.h"

struct PipelineConfigInfo 
{

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

    static PipelineConfigInfo DefaultPipelineConfigInfo();
private:
    static std::vector<char> ReadFile(const std::string& filepath);

    void CreateGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo);

    void CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule);

    Device& m_Device;
    VkPipeline m_Pipeline;
    VkShaderModule m_VertexShaderModule;
    VkShaderModule m_FragmentShaderModule;
};