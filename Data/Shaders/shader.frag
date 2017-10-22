// temporarily changed from block because glslang has a bug
layout (location = 0) in vec3 VertNorm;
layout (location = 1) in vec2 VertUV;
layout (location = 2) in vec3 VertPosView;

#if HAS_NORMAL == 1
layout (location = 3) in vec3 VertTangentView;
layout (location = 4) in vec3 VertBinormView;
#endif // HAS_NORMAL == 1


layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform lightcb
{
    vec4 pos;
    vec4 diffuse;
} lightCBuffer;

layout (set = 2, binding = 0) uniform sampler2D diffTex;
layout (set = 3, binding = 0) uniform sampler2D normTex;
layout (set = 4, binding = 0) uniform sampler2D maskTex;

vec4 lightAmbient = vec4(0.3, 0.3, 0.3, 1.0);

void main()
{
#if HAS_COLOR_MASK == 1
    float mask = texture(maskTex, VertUV).r;
    if (mask.r < 0.5)
        discard;
#endif // HAS_COLOR_MASK == 1

    color = lightAmbient;
    vec3 lightDir = lightCBuffer.pos.xyz - VertPosView;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);

#if HAS_NORMAL == 1
    mat3 TBN = transpose(mat3(VertTangentView, VertBinormView, VertNorm));
    lightDir = TBN * lightDir;
    float coeff = dot(lightDir, normalize((texture(normTex, VertUV).rgb * 2.0) - 1.0));
#else // HAS_NORMAL == 1
    float coeff = dot(lightDir, VertNorm);
#endif // HAS_NORMAL == 1

    color += coeff * lightCBuffer.diffuse * (5.0 / (distance * distance));

#if HAS_TEXTURE == 1
    color *= texture(diffTex, VertUV);
#else // HAS_TEXTURE == 1
    color *= vec4(0.65, 0.36, 0.13, 1.0);
#endif // HAS_TEXTURE == 1
}
