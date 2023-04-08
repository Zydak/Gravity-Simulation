#pragma once

#include "window.h"
#include "camera.h"

class CameraController
{
public:
    CameraController(GLFWwindow* window);
    ~CameraController();

    void Update(float delta, Camera& camera, glm::vec3 target);
private:
    GLFWwindow* m_Window;
};