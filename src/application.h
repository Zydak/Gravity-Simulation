#pragma once
#include "window.h"
#include "pipeline.h"
#include "device.h"

class Application
{
public:
    Application();
    ~Application();

    static constexpr int WIDTH = 800;
    static constexpr int HEIGHT = 800;

    void Run();
private:
    Window m_Window{WIDTH, HEIGHT, "Gravity"};
    Device m_Device{m_Window};
    Pipeline m_Pipeline{"shaders/shader.vert.spv", "shaders/shader.frag.spv"};
};