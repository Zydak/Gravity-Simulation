#version 450

const vec2 OFFSETS[6] = vec2[]
(
    vec2(-1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0, -1.0),
    vec2( 1.0, -1.0),
    vec2(-1.0,  1.0),
    vec2( 1.0,  1.0)
);

const vec2 TEXOFFSETS[6] = vec2[]
(
    vec2(0.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 0.0),
    vec2(1.0, 0.0),
    vec2(0.0, 1.0),
    vec2(1.0, 1.0)
);

layout (location = 0) out vec2 fragOffset;
layout (location = 1) out vec3 outColor;
layout (location = 2) out vec2 texOffset;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout(push_constant) uniform Push
{
    vec3 position;
    vec3 color;
    vec3 offset;
    float radius;
} push;

void main()
{
    fragOffset = OFFSETS[gl_VertexIndex];
    texOffset = TEXOFFSETS[gl_VertexIndex];
    outColor = push.color;
    vec4 cameraSpace = ubo.view * vec4(push.position-push.offset, 1.0);
    vec4 positionInCameraSpace = cameraSpace + push.radius * vec4(fragOffset, 0.0, 0.0);
    vec4 pos = ubo.projection * positionInCameraSpace;
    gl_Position = pos.xyww;
}