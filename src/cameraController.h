#pragma once

#include "window.h"
#include "camera.h"

class CameraController
{
public:
    CameraController(GLFWwindow* window);
    ~CameraController();

    void Update(Camera& camera);
private:
    GLFWwindow* m_Window;
};