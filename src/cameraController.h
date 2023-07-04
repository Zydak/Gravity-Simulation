#pragma once

#include "vulkan/window.h"
#include "camera.h"
#include "frameInfo.h"

class CameraController
{
public:
    CameraController(GLFWwindow* window);
    ~CameraController();

    void Update(const float& delta, Camera& camera, const int& target, Map& gameObjects, bool inputOn);
private:
    GLFWwindow* m_Window;
};