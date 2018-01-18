layout (location = 0) in vec3 InPos;
layout (location = 1) in vec3 InNorm;
layout (location = 2) in vec2 InUV;
layout (location = 3) in vec3 InTangent;

layout (location = 0) out vec3 VertNorm;
layout (location = 1) out vec2 VertUV;
layout (location = 2) out vec3 VertPosWorld;
#if HAS_NORMAL == 1
layout (location = 3) out vec3 VertTang;
layout (location = 4) out vec3 VertBitang;
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

    VertNorm = normalize(mat3(dynamicCBuffer.worldMatrix) * InNorm);
    VertUV = InUV;
    gl_Position = worldViewProj * vec4(InPos, 1.0);

    VertPosWorld = mat3(dynamicCBuffer.worldMatrix) * InPos;

#if HAS_NORMAL == 1
    VertTang = normalize(mat3(dynamicCBuffer.worldMatrix) * InTangent);
    VertBitang = normalize(mat3(dynamicCBuffer.worldMatrix) * cross(VertTang, VertNorm));
#endif // HAS_NORMAL == 1
}
