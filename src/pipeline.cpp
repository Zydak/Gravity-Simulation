#include "pipeline.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

Pipeline::Pipeline(Device& device, 
        const std::string& vertexPath, 
        const std::string& fragmentPath, 
        const PipelineConfigInfo& configInfo)
        : m_Device(device)
{
    CreateGraphicsPipeline(vertexPath, fragmentPath, configInfo);
}

Pipeline::~Pipeline()
{
    vkDestroyShaderModule(m_Device.GetDevice(), m_VertexShaderModule, nullptr);
    vkDestroyShaderModule(m_Device.GetDevice(), m_FragmentShaderModule, nullptr);
}

std::vector<char> Pipeline::ReadFile(const std::string& filepath)
{
    std::ifstream file(filepath, std::ios::ate | std::ios::binary); // ate goes to the end of the file so reading filesize is easier and binary avoids text transformation
    if (!file.is_open())
    {
        throw std::runtime_error("failed to open file: " + filepath);
    }

    size_t fileSize = (size_t)(file.tellg()); // tellg gets current position in file
    std::vector<char> buffer(fileSize);
    file.seekg(0); // return to the begining of the file
    file.read(buffer.data(), fileSize);

    file.close();
    return buffer;
}

void Pipeline::CreateGraphicsPipeline(const std::string& vertexPath, const std::string& fragmentPath, const PipelineConfigInfo& configInfo)
{
    auto vertCode = ReadFile(vertexPath);
    auto fragCode = ReadFile(fragmentPath);

    std::cout << "Vertex Code Size: " << vertCode.size() << std::endl;
    std::cout << "Fragment Code Size: " << fragCode.size() << std::endl;

    CreateShaderModule(vertCode, &m_VertexShaderModule);
    CreateShaderModule(fragCode, &m_FragmentShaderModule);

    VkPipelineShaderStageCreateInfo shaderStages[2];
    shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
    shaderStages[0].module = m_VertexShaderModule;
    shaderStages[0].pName = "main";
    shaderStages[0].flags = 0;
    shaderStages[0].pNext = nullptr;
    shaderStages[0].pSpecializationInfo = nullptr;

    shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    shaderStages[1].module = m_FragmentShaderModule;
    shaderStages[1].pName = "main";
    shaderStages[1].flags = 0;
    shaderStages[1].pNext = nullptr;
    shaderStages[1].pSpecializationInfo = nullptr;
}

void Pipeline::CreateShaderModule(const std::vector<char>& code, VkShaderModule* shaderModule)
{
    VkShaderModuleCreateInfo createInfo{};

    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size();
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

    if (vkCreateShaderModule(m_Device.GetDevice(), &createInfo, nullptr, shaderModule) != VK_SUCCESS)
    {
        throw std::runtime_error("failed to create shader module");
    }
}

PipelineConfigInfo Pipeline::DefaultPipelineConfigInfo()
{
    PipelineConfigInfo configInfo{};

    return configInfo;
}