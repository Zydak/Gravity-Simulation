#pragma once

#include "device.h"
#include "cubemap.h"
#include "models/skyboxModel.h"

#include <memory>

#define SKYBOX_MILKY_WAY 0
#define SKYBOX_NEBULA 1
#define SKYBOX_STARS 2
#define SKYBOX_RED_GALAXY 3

class Skybox
{
public:
    Skybox(Device& device, uint32_t image);
    ~Skybox() = default;

    inline Cubemap& GetCubemap() { return m_Cubemap; }
    inline SkyboxModel* GetSkyboxModel() { return m_SkyboxModel.get(); }

private:
    Device& m_Device;
    std::unique_ptr<SkyboxModel> m_SkyboxModel;

    Cubemap m_Cubemap{m_Device};
};