#include "skybox.h"
#include "../defines.h"

Skybox::Skybox(Device& device, uint32_t image)
    : m_Device(device)
{
    m_SkyboxModel = SkyboxModel::CreateModelFromFile(m_Device, "assets/models/cube.obj");

    std::array<std::string, 6> filepaths{};
    #ifndef FAST_LOAD
    switch(image)
    {
        case SkyboxTextureImage::MilkyWay:
            filepaths[0] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_NegativeZ.png";
            filepaths[1] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_PositiveZ.png";
            filepaths[2] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_PositiveY.png";
            filepaths[3] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_NegativeY.png";
            filepaths[4] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_PositiveX.png";
            filepaths[5] = "assets/textures/cubemaps/milkyWay/MilkyWayTex_NegativeX.png";
            break;
        case SkyboxTextureImage::Nebula:
            filepaths[0] = "assets/textures/cubemaps/nebula/NebulaTex_NegativeZ.png";
            filepaths[1] = "assets/textures/cubemaps/nebula/NebulaTex_PositiveZ.png";
            filepaths[2] = "assets/textures/cubemaps/nebula/NebulaTex_PositiveY.png";
            filepaths[3] = "assets/textures/cubemaps/nebula/NebulaTex_NegativeY.png";
            filepaths[4] = "assets/textures/cubemaps/nebula/NebulaTex_PositiveX.png";
            filepaths[5] = "assets/textures/cubemaps/nebula/NebulaTex_NegativeX.png";
            break;
        case SkyboxTextureImage::Stars:
            filepaths[0] = "assets/textures/cubemaps/stars/StarsTex_NegativeZ.png";
            filepaths[1] = "assets/textures/cubemaps/stars/StarsTex_PositiveZ.png";
            filepaths[2] = "assets/textures/cubemaps/stars/StarsTex_PositiveY.png";
            filepaths[3] = "assets/textures/cubemaps/stars/StarsTex_NegativeY.png";
            filepaths[4] = "assets/textures/cubemaps/stars/StarsTex_PositiveX.png";
            filepaths[5] = "assets/textures/cubemaps/stars/StarsTex_NegativeX.png";
            break;
        case SkyboxTextureImage::RedGalaxy:
            filepaths[0] = "assets/textures/cubemaps/redGalaxy/GalaxyTex_NegativeZ.png";
            filepaths[1] = "assets/textures/cubemaps/redGalaxy/GalaxyTex_PositiveZ.png";
            filepaths[2] = "assets/textures/cubemaps/redGalaxy/GalaxyTex_PositiveY.png";
            filepaths[3] = "assets/textures/cubemaps/redGalaxy/GalaxyTex_NegativeY.png";
            filepaths[4] = "assets/textures/cubemaps/redGalaxy/GalaxyTex_PositiveX.png";
            filepaths[5] = "assets/textures/cubemaps/redGalaxy/GalaxyTex_NegativeX.png";
            break;
    }
    #else
    filepaths[0] = "assets/textures/black.png";
    filepaths[1] = "assets/textures/black.png";
    filepaths[2] = "assets/textures/black.png";
    filepaths[3] = "assets/textures/black.png";
    filepaths[4] = "assets/textures/black.png";
    filepaths[5] = "assets/textures/black.png";
    #endif

    m_Cubemap.CreateImageFromTexture(filepaths);
}