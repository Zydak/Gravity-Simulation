#include "pipeline.h"

#include <fstream>
#include <stdexcept>
#include <iostream>

Pipeline::Pipeline(const std::string& vertexfilepath, const std::string& fragmentfilepath)
{
    CreateGraphicsPipeline(vertexfilepath, fragmentfilepath);
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

void Pipeline::CreateGraphicsPipeline(const std::string& vertexfilepath, const std::string& fragmentfilepath)
{
    auto vertCode = ReadFile(vertexfilepath);
    auto fragCode = ReadFile(fragmentfilepath);

    std::cout << "Vertex Code Size: " << vertCode.size() << std::endl;
    std::cout << "Fragment Code Size: " << fragCode.size() << std::endl;
}