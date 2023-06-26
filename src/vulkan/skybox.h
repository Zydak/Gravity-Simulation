#pragma once

#include "device.h"
#include "cubemap.h"
#include "../models/customModelPosOnly.h"

#include <memory>

enum SkyboxTextureImage
{
    MilkyWay = 0,
    Nebula = 1,
    Stars = 2,
    RedGalaxy = 3
};

class Skybox
{
public:
    Skybox(Device& device, uint32_t image);
    ~Skybox() = default;

    inline Cubemap& GetCubemap() { return m_Cubemap; }
    inline CustomModelPosOnly* GetSkyboxModel() { return m_SkyboxModel.get(); }

private:
    Device& m_Device;
    std::unique_ptr<CustomModelPosOnly> m_SkyboxModel;
    glm::mat4 m_ModelTransform;

    Cubemap m_Cubemap{m_Device};
};