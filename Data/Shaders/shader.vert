layout (location = 0) in vec3 InPos;
layout (location = 1) in vec3 InNorm;
layout (location = 2) in vec2 InUV;
layout (location = 3) in vec3 InTangent;



// temporarily changed from block because glslang has a bug
layout (location = 0) out vec3 VertNorm;
layout (location = 1) out vec2 VertUV;
layout (location = 2) out vec3 VertPosView;

#if HAS_NORMAL == 1
layout (location = 3) out vec3 VertTangentView;
layout (location = 4) out vec3 VertBinormView;
#endif // HAS_NORMAL == 1



layout (set = 0, binding = 0) uniform dynamicCb
{
    mat4 worldMatrix;
} dynamicCBuffer;

layout (set = 0, binding = 1) uniform cb
{
    mat4 viewMatrix;
    mat4 projMatrix;
} CBuffer;


out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
    mat4 worldView = CBuffer.viewMatrix * dynamicCBuffer.worldMatrix;
    mat4 worldViewProj = CBuffer.projMatrix * worldView;

    VertNorm = InNorm;
    VertUV = InUV;
    VertPosView = mat3(dynamicCBuffer.worldMatrix) * InPos;
    gl_Position = worldViewProj * vec4(InPos, 1.0);

#if HAS_NORMAL == 1
    VertTangentView = normalize(mat3(dynamicCBuffer.worldMatrix) * InTangent);
    VertBinormView = normalize(mat3(dynamicCBuffer.worldMatrix) * cross(InTangent, InNorm));
#endif // HAS_NORMAL == 1
}
