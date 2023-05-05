#pragma once
#include "vulkan/window.h"
#include "vulkan/device.h"
#include "object.h"
#include "renderer.h"
#include "camera.h"
#include "cameraController.h"
#include "vulkan/descriptors.h"
#include "vulkan/skybox.h"

#include <iostream>
#include <memory>
#include <vector>
#include <chrono>

class Application
{
public:
    Application();
    ~Application();

    void Run();
private:
    void LoadGameObjects();

    void Update(const FrameInfo& frameInfo, float delta);

    void RenderImGui(const FrameInfo& frameInfo);

    Window m_Window{1600, 900, "Gravity"};
    Device m_Device{m_Window};
    Camera m_Camera{};
    std::unique_ptr<Renderer> m_Renderer;
    CameraController m_CameraController{m_Window.GetGLFWwindow()};

    std::unique_ptr<DescriptorPool> m_GlobalPool{};
    Map m_GameObjects;

    Sampler m_Sampler{m_Device};
    std::vector<std::unique_ptr<Buffer>> m_UboBuffers;

    std::unique_ptr<Skybox> m_Skybox;
    VkDescriptorSet m_SkyboxDescriptorSet{};
    std::unique_ptr<DescriptorSetLayout> m_SkyboxSetLayout;
private:
    float m_MainLoopAccumulator = 0;
    float m_FPSaccumulator = 0;
    float m_FPS = 0;
    uint32_t m_TargetLock = 0;
    int m_StepCount = 1; // TODO: fix step count, when step count is high float starts to break because we're dividing 0.016 by something like 2500
    int m_GameSpeed = 1;
    bool m_Pause = true;
    glm::dvec3 m_Offset;
    TextureImage m_IconImage{m_Device, "assets/textures/icon.png"};
};