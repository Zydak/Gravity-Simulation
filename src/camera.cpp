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

void Camera::SetOrthographicProjection(float left, float right, float top, float bottom, float near, float far) 
{
    m_ProjectionMatrix = glm::mat4{1.0f};
    m_ProjectionMatrix[0][0] = 2.f / (right - left);
    m_ProjectionMatrix[1][1] = 2.f / (bottom - top);
    m_ProjectionMatrix[2][2] = 1.f / (far - near);
    m_ProjectionMatrix[3][0] = -(right + left) / (right - left);
    m_ProjectionMatrix[3][1] = -(bottom + top) / (bottom - top);
    m_ProjectionMatrix[3][2] = -near / (far - near);
}

void Camera::SetViewDirection(glm::vec3 direction, glm::dvec3 up) 
{
    const glm::dvec3 w{glm::normalize(direction)};
    const glm::dvec3 u{glm::normalize(glm::cross(w, up))};
    const glm::dvec3 v{glm::cross(w, u)};

    m_ViewMatrix = glm::mat4{1.f};
    m_ViewMatrix[0][0] = u.x;
    m_ViewMatrix[1][0] = u.y;
    m_ViewMatrix[2][0] = u.z;
    m_ViewMatrix[0][1] = v.x;
    m_ViewMatrix[1][1] = v.y;
    m_ViewMatrix[2][1] = v.z;
    m_ViewMatrix[0][2] = w.x;
    m_ViewMatrix[1][2] = w.y;
    m_ViewMatrix[2][2] = w.z;
    m_ViewMatrix[3][0] = -glm::dot(u, m_Transform.translation);
    m_ViewMatrix[3][1] = -glm::dot(v, m_Transform.translation);
    m_ViewMatrix[3][2] = -glm::dot(w, m_Transform.translation);
}

void Camera::SetViewTarget(glm::dvec3 target, glm::dvec3 up) 
{
    SetViewDirection(target - m_Transform.translation, up);
}

void Camera::SetViewYXZ(glm::vec3& rotation) 
{
    const float c3 = glm::cos(rotation.z);
    const float s3 = glm::sin(rotation.z);
    const float c2 = glm::cos(rotation.x);
    const float s2 = glm::sin(rotation.x);
    const float c1 = glm::cos(rotation.y);
    const float s1 = glm::sin(rotation.y);
    const glm::dvec3 u{(c1 * c3 + s1 * s2 * s3), (c2 * s3), (c1 * s2 * s3 - c3 * s1)};
    const glm::dvec3 v{(c3 * s1 * s2 - c1 * s3), (c2 * c3), (c1 * c3 * s2 + s1 * s3)};
    const glm::dvec3 w{(c2 * s1), (-s2), (c1 * c2)};
    m_ViewMatrix = glm::mat4{1.f};
    m_ViewMatrix[0][0] = u.x;
    m_ViewMatrix[1][0] = u.y;
    m_ViewMatrix[2][0] = u.z;
    m_ViewMatrix[0][1] = v.x;
    m_ViewMatrix[1][1] = v.y;
    m_ViewMatrix[2][1] = v.z;
    m_ViewMatrix[0][2] = w.x;
    m_ViewMatrix[1][2] = w.y;
    m_ViewMatrix[2][2] = w.z;
    m_ViewMatrix[3][0] = -glm::dot(u, m_Transform.translation);
    m_ViewMatrix[3][1] = -glm::dot(v, m_Transform.translation);
    m_ViewMatrix[3][2] = -glm::dot(w, m_Transform.translation);
}