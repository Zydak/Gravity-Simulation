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

layout (location = 0) out vec2 fragOffset;

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
    float radius;
} push;

void main()
{
    // fragOffset = OFFSETS[gl_VertexIndex];
    // vec3 cameraRightWorld = {ubo.view[0][0], ubo.view[1][0], ubo.view[2][0]};
    // vec3 cameraUpWorld = {ubo.view[0][1], ubo.view[1][1], ubo.view[2][1]};

    // vec3 positionWorld = push.position + push.radius * fragOffset.x * cameraRightWorld
    //     + ubo.projection * fragOffset.y * cameraUpWorld;

    // mat4 proj = mat4(1.0);
    // gl_Position = push.ortho * ubo.view * vec4(positionWorld, 1.0);

    fragOffset = OFFSETS[gl_VertexIndex];
    vec4 cameraSpace = ubo.view * vec4(push.position, 1.0);
    vec4 positionInCameraSpace = cameraSpace + push.radius * vec4(fragOffset, 0.0, 0.0);
    gl_Position = ubo.projection * positionInCameraSpace;
}