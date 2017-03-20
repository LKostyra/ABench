#version 450
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable

layout (location = 0) in vec3 InPos;
layout (location = 1) in vec4 InColor;

out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out VertexShaderOutput
{
    vec4 Color;
} Output;

layout (std140, set = 0, binding = 0) uniform cb
{
	mat4 viewMatrix;
	mat4 projMatrix;
} cbuffer;

void main()
{
    gl_Position = cbuffer.viewMatrix * vec4(InPos, 1.0);
    Output.Color = InColor;
}
