#pragma once

#include <string>
#include <vector>


class Pipeline
{
public:
    Pipeline(const std::string& vertexfilepath, const std::string& fragmentfilepath);

    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
private:
    static std::vector<char> ReadFile(const std::string& filepath);

    void CreateGraphicsPipeline(const std::string& vertexfilepath, const std::string& fragmentfilepath);
};