#version 450
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable


layout (location = 0) in vec3 InPos;
layout (location = 1) in vec3 InNorm;
layout (location = 2) in vec2 InUV;


out gl_PerVertex
{
    vec4 gl_Position;
};

layout (location = 0) out VertexShaderOutput
{
    vec3 Norm;
    vec2 UV;
} Output;


layout (std140, set = 0, binding = 0) uniform dynamicCb
{
    mat4 worldMatrix;
} dynamicCBuffer;

layout (std140, set = 0, binding = 1) uniform cb
{
    mat4 viewMatrix;
    mat4 projMatrix;
} CBuffer;


void main()
{
    mat4 worldView = CBuffer.viewMatrix;// * dynamicCBuffer.worldMatrix;
    mat4 worldViewProj = CBuffer.projMatrix * worldView;
    gl_Position = worldViewProj * vec4(InPos, 1.0);
    Output.Norm = InNorm;
    Output.UV = InUV;
}
