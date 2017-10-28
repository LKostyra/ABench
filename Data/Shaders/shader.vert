layout (location = 0) in vec3 InPos;
layout (location = 1) in vec3 InNorm;
layout (location = 2) in vec2 InUV;
layout (location = 3) in vec3 InTangent;



// temporarily changed from block because glslang has a bug
layout (location = 0) out vec3 VertNorm;
layout (location = 1) out vec2 VertUV;
layout (location = 2) out vec3 VertLightDir;



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

layout (set = 1, binding = 0) uniform lightcb
{
    vec4 pos;
    vec4 diffuse;
} lightCBuffer;

void main()
{
    mat4 worldView = CBuffer.viewMatrix * dynamicCBuffer.worldMatrix;
    mat4 worldViewProj = CBuffer.projMatrix * worldView;

    VertNorm = normalize(mat3(dynamicCBuffer.worldMatrix) * InNorm);
    VertUV = InUV;
    gl_Position = worldViewProj * vec4(InPos, 1.0);

    vec3 vertPos = mat3(dynamicCBuffer.worldMatrix) * InPos;
    VertLightDir = lightCBuffer.pos.xyz - vertPos;

#if HAS_NORMAL == 1
    vec3 tangent = normalize(mat3(dynamicCBuffer.worldMatrix) * InTangent);
    vec3 bitangent = normalize(mat3(dynamicCBuffer.worldMatrix) * cross(tangent, VertNorm));

    mat3 TBN = transpose(mat3(tangent, bitangent, VertNorm));
    VertLightDir = TBN * VertLightDir;
#endif // HAS_NORMAL == 1
}
