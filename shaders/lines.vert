#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout(location = 0) out vec3 fragColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

void main()
{
    vec4 positionWorld = vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    fragColor = vec3(1.0f, 1.0f, 1.0f);
}