#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 texCoord;
layout(location = 4) in vec2 uv;

layout(location = 0) out vec3 fragColor;
layout(location = 1) out vec3 fragPosWorld;
layout(location = 2) out vec3 fragNormalWorld;
layout(location = 3) out vec2 fragTexCoord;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
    vec3 offset;
} push;

void main()
{
    mat4 modelMat = push.modelMatrix;
    modelMat[3][0] -= push.offset.x;
    modelMat[3][1] -= push.offset.y;
    modelMat[3][2] -= push.offset.z;
    vec4 positionWorld = modelMat * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;

    fragNormalWorld = normalize(mat3(push.modelMatrix) * normal);
    fragPosWorld = positionWorld.xyz;
    fragColor = color;
    fragTexCoord = texCoord;
}