#version 450

layout(location = 0) in vec3 position;

layout(location = 0) out vec3 color;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push
{
    vec3 offset;
    vec3 color;
} push;

void main()
{
    vec3 pos = position - push.offset;
    vec4 positionWorld = vec4(pos, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    color = push.color;
}