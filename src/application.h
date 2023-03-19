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
    Window m_Window;
    Pipeline m_Pipeline;
    Device m_Device;
};