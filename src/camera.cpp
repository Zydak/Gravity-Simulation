#include "camera.h"

#include <cassert>
#include <limits>
 
void Camera::SetPerspectiveProjection(float fov, float aspect, float near, float far) 
{
    assert(glm::abs(aspect - std::numeric_limits<float>::epsilon()) > 0.0f);
    const float tanHalfFov = tan(fov / 2.f);
    m_ProjectionMatrix = glm::mat4{0.0f};
    m_ProjectionMatrix[0][0] = 1.f / (aspect * tanHalfFov);
    m_ProjectionMatrix[1][1] = 1.f / (tanHalfFov);
    m_ProjectionMatrix[2][2] = far / (far - near);
    m_ProjectionMatrix[2][3] = 1.f;
    m_ProjectionMatrix[3][2] = -(far * near) / (far - near);
}