#version 450

layout(location = 0) in vec3 fragColor;
layout(location = 1) in vec3 fragPosWorld;
layout(location = 2) in vec3 fragNormalWorld;
layout(location = 3) in vec2 fragTexCoord;

layout (location = 0) out vec4 outColor;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

//layout(set = 1, binding = 0) uniform sampler2D texSampler;

void main()
{
    vec3 directionToLight = ubo.lightPosition - fragPosWorld;
    vec3 lightColor = ubo.lightColor.xyz * ubo.lightColor.w;

    vec3 diffuseLight = lightColor * max(dot(normalize(fragNormalWorld), normalize(directionToLight)), 0);

    outColor = vec4(vec3(1.0, 1.0, 1.0) * diffuseLight * fragColor, 1.0);
}