#version 450

layout (location = 0) in vec2 fragOffset;
layout (location = 1) in vec3 inColor;
layout (location = 2) in vec2 texOffset;
layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout (set = 0, binding = 1) uniform sampler2D texSampler;

void main()
{
    outColor = texture(texSampler, texOffset) * vec4(inColor, 1.0);
}