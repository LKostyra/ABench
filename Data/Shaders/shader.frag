#version 450
#extension GL_ARB_separate_shader_objects: enable
#extension GL_ARB_shading_language_420pack: enable

layout (location = 0) in VertexShaderOutput
{
    vec3 Norm;
} Input;

layout (location = 0) out vec4 color;

void main()
{
    // treat normal vector as color for now
    // translate -1..1 coords to 0..1
    color = vec4((Input.Norm + 1.0) / 2.0, 1.0);
}
