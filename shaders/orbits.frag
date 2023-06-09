#version 450

layout (location = 0) in vec3 inColor;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

void main()
{
    outColor = vec4(inColor, 1.0f);
}