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

    Window m_Window{800, 600, "Gravity"};
    Device m_Device{m_Window};
    Camera m_Camera{};
    std::unique_ptr<Renderer> m_Renderer;
    CameraController m_CameraController{m_Window.GetGLFWwindow()};

    std::unique_ptr<DescriptorPool> m_GlobalPool{};
    Map m_GameObjects;
    Map m_Lines;
};