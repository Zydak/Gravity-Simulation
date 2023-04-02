#include "cameraController.h"

static float yaw = 0;
static float pitch = 0;
static float lastX;
static float lastY;
static bool canClick = false;

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
            canClick = true;
        }
        else if(GLFW_RELEASE == action)
        {
            canClick = false;
        }
    }
}
static double scrollY = 10;
void scrollCallback(GLFWwindow* window, double xoffset, double yoffset)
{
    scrollY -= yoffset;
    if (scrollY < 5)
        scrollY = 5;
}

CameraController::CameraController(GLFWwindow* window)
    : m_Window(window)
{
    glfwSetScrollCallback(m_Window, scrollCallback);
    glfwSetMouseButtonCallback(m_Window, mouseCallback);
}

CameraController::~CameraController()
{
    
}

void CameraController::Update(Camera& camera)
{
    float radius = scrollY;
    if (glfwGetMouseButton(m_Window, 0) == GLFW_PRESS)
    {
        double x, y;
        glfwGetCursorPos(m_Window, &x, &y);
            
        static float xOffset = 0;
        static float yOffset = 0;
        xOffset = (lastX - x) * -1.0f;
        yOffset = (lastY - y) * -1.0f;
        yaw += xOffset;
        pitch += yOffset;

        if (pitch > 89)
            pitch = 89;
        if (pitch < -89)
            pitch = -89;

        lastX = x;
        lastY = y;
    }
        
    camera.m_Transform.translation.x = radius * -sinf(yaw*(M_PI/180)) * cosf((pitch)*(M_PI/180));
    camera.m_Transform.translation.y = radius * -sinf((pitch)*(M_PI/180));
    camera.m_Transform.translation.z = -radius * cosf((yaw)*(M_PI/180)) * cosf((pitch)*(M_PI/180));
    camera.m_Transform.rotation.x = yaw;
    camera.m_Transform.rotation.y = pitch;
}