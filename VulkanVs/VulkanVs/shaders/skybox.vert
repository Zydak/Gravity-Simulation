#version 450

layout(location = 0) in vec3 position;

layout(set = 0, binding = 0) uniform GlobalUbo
{
    mat4 projection;
    mat4 view;
    vec3 lightPosition;
    vec4 lightColor;
} ubo;

layout (location = 0) out vec3 outUVW;

layout(push_constant) uniform Push
{
    mat4 modelMatrix;
} push;

void main() 
{
	outUVW = position;

	vec4 positionWorld = push.modelMatrix * vec4(position, 1.0);
    gl_Position = ubo.projection * ubo.view * positionWorld;
}
