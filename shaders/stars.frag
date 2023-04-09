#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main()
{
    outColor = vec4(texture(texSampler, fragTexCoord).rgb * fragColor, 1.0) * ubo.lightColor;
}