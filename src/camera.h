#pragma once

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

class Camera
{
public:
    void SetPerspectiveProjection(float fov, float aspect, float near, float far);

    inline const glm::mat4& GetProjection() const { return m_ProjectionMatrix; }
private:
    glm::mat4 m_ProjectionMatrix{1.0f};
};