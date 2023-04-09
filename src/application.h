#pragma once
#include "window.h"
#include "pipeline.h"
#include "device.h"
#include "pipeline.h"
#include "swapchain.h"
#include "object.h"
#include "renderer.h"
#include "camera.h"
#include "cameraController.h"
#include "buffer.h"
#include "descriptors.h"
#include "models/simpleModel.h"
#include "textureImage.h"
#include "sampler.h"

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
    void Update(FrameInfo frameInfo, float delta, uint32_t substeps);
    void RenderImGui(FrameInfo frameInfo);

    Window m_Window{1600, 900, "Gravity"};
    Device m_Device{m_Window};
    Camera m_Camera{};
    std::unique_ptr<Renderer> m_Renderer;
    CameraController m_CameraController{m_Window.GetGLFWwindow()};

    std::unique_ptr<DescriptorPool> m_GlobalPool{};
    Map m_GameObjects;
    std::unique_ptr<SimpleModel> m_Obj;
    
    TextureImage m_Texture{m_Device, "assets/textures/viking_room.png"};
    Sampler m_Sampler{m_Device};

private:
    float accumulator = 0;
    float FPSaccumulator = 0;
    float FPS = 0;
    int TargetLock = 0;
    int stepCount = 1;
    int gameSpeed = 1;
};