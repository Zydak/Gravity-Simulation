#include "cameraController.h"

#include <iostream>

#include "imgui.h"

static double yaw = 0;
static double pitch = 89;
static double lastX;
static double lastY;

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
static double scrollY = 0.00000000000000001;
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0)
    	scrollY -= scrollY / 4;
	if (yoffset < 0)
    	scrollY += scrollY / 4;

    if (scrollY >= 825142)
        scrollY = 825142;
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
void CameraController::Update(const float& delta, Camera& camera, const int& target, Map& gameObjects, bool inputOn)
{
    if (scrollY < gameObjects[target]->GetObjectProperties().radius*2/SCALE_DOWN)
        scrollY = gameObjects[target]->GetObjectProperties().radius*2/SCALE_DOWN;
    static double radius;
    if (!inputOn)
    {
        scrollY = radius;
    }
    radius = scrollY;
    //if (glfwGetMouseButton(m_Window, 0) == GLFW_PRESS)
    if (glfwGetMouseButton(m_Window, 0) == GLFW_PRESS && inputOn)
    {
        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
            
        double sensitivity = -10.0f;
        static double xOffset = 0;
        static double yOffset = 0;
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
    camera.m_Transform.translation.x = radius * -sin(yaw*(M_PI/180.0)) * cos((pitch)*(M_PI/180.0));
    camera.m_Transform.translation.y = radius * -sin((pitch)*(M_PI/180.0));
    camera.m_Transform.translation.z = -radius * cos((yaw)*(M_PI/180.0)) * cos((pitch)*(M_PI/180.0));

    // only for debuging info
    camera.m_Transform.rotation.x = yaw;
    camera.m_Transform.rotation.y = pitch;
}