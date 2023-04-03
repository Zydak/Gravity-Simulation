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

    Window m_Window{800, 600, "Gravity"};
    Device m_Device{m_Window};
    Camera m_Camera{};
    CameraController m_CameraController{m_Window.GetGLFWwindow()};

    std::unique_ptr<DescriptorPool> m_GlobalPool{};
    std::vector<std::shared_ptr<Object>> m_GameObjects;
};