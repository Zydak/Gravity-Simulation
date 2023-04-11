#include "cameraController.h"

#include <iostream>

#include "imgui.h"

static float yaw = 0;
static float pitch = 89;
static float lastX;
static float lastY;

static void mouseCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT) 
    {
        double x, y;
        glfwGetCursorPos(window, &x, &y);
        if(GLFW_PRESS == action)
        {
            lastX = x;
            lastY = y;
        }
        else if(GLFW_RELEASE == action)
        {
        }
    }
}
static double scrollY = 500;
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0)
    	scrollY += yoffset - scrollY/4;
	if (yoffset < 0)
    	scrollY -= yoffset - scrollY/4;

    if (scrollY < 5)
        scrollY = 5;
}

CameraController::CameraController(GLFWwindow* window)
    : m_Window(window)
{
    glfwSetScrollCallback(m_Window, ScrollCallback);
    glfwSetMouseButtonCallback(m_Window, mouseCallback);
}

CameraController::~CameraController()
{
    
}

/*
    * @brief Updates camera position based on mouse movement
*/
void CameraController::Update(float delta, Camera& camera, glm::vec3 target)
{
    float radius = scrollY;
    if (glfwGetMouseButton(m_Window, 0) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
    {
        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
            
        float sensitivity = -25.0f;
        static float xOffset = 0;
        static float yOffset = 0;
        xOffset = (lastX - x) * sensitivity;
        yOffset = (lastY - y) * sensitivity;
        yaw += xOffset * delta;
        pitch += yOffset * delta;

        if (pitch > 89)
            pitch = 89;
        if (pitch < -89)
            pitch = -89;

        lastX = x;
        lastY = y;
    }
    
    // Equation for camera positioning around a sphere
    camera.m_Transform.translation.x = target.x + radius * -sinf(yaw*(M_PI/180)) * cosf((pitch)*(M_PI/180));
    camera.m_Transform.translation.y = target.y + radius * -sinf((pitch)*(M_PI/180));
    camera.m_Transform.translation.z = target.z + -radius * cosf((yaw)*(M_PI/180)) * cosf((pitch)*(M_PI/180));

    // only for debuging info
    camera.m_Transform.rotation.x = yaw;
    camera.m_Transform.rotation.y = pitch;
}