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
static double scrollY = 5000;
void ScrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
	if (yoffset > 0)
    	scrollY += yoffset - scrollY/4;
	if (yoffset < 0)
    	scrollY -= yoffset - scrollY/4;

    //if (scrollY <= 10)
    //    scrollY = 10;

    if (scrollY >= 1.24701e+09)
        scrollY = 1.24701e+09;
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
void CameraController::Update(const float& delta, Camera& camera, const int& target, Map& gameObjects)
{
    glm::vec3 targetPos = gameObjects[target]->GetObjectTransform().translation/SCALE_DOWN;
    if (scrollY < gameObjects[target]->GetObjectProperties().radius*2/SCALE_DOWN)
        scrollY = gameObjects[target]->GetObjectProperties().radius*2/SCALE_DOWN;
    float radius = scrollY;
    if (glfwGetMouseButton(m_Window, 0) == GLFW_PRESS && !ImGui::GetIO().WantCaptureMouse)
    {
        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
            
        float sensitivity = -10.0f;
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
    camera.m_Transform.translation.x = targetPos.x + radius * -sinf(yaw*(M_PI/180)) * cosf((pitch)*(M_PI/180));
    camera.m_Transform.translation.y = targetPos.y + radius * -sinf((pitch)*(M_PI/180));
    camera.m_Transform.translation.z = targetPos.z + -radius * cosf((yaw)*(M_PI/180)) * cosf((pitch)*(M_PI/180));

    // only for debuging info
    camera.m_Transform.rotation.x = yaw;
    camera.m_Transform.rotation.y = pitch;
}