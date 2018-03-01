layout (early_fragment_tests) in;


layout (location = 0) in vec3 VertNorm;
layout (location = 1) in vec2 VertUV;
layout (location = 2) in vec3 VertPosWorld;
#if HAS_NORMAL == 1
layout (location = 3) in vec3 VertTang;
layout (location = 4) in vec3 VertBitang;
#endif // HAS_NORMAL == 1

layout (location = 0) out vec4 color;


// light-related structures
struct LightData
{
    vec4 pos;
    vec3 diffuse;
    float range;
};

struct GridLight
{
    uint offset;
    uint count;
    uvec2 padding;
};


// Set numbering starts from 1 because Set binding slots are common with VS
// light-related buffers
layout (set = 1, binding = 0) uniform _lightParams
{
    uvec2 viewport;
    uint pixelsPerViewFrustum;
} lightParams;

layout (set = 1, binding = 1) uniform _materialParams
{
    vec4 color;
} materialParams;

layout (set = 1, binding = 2) buffer _lightData
{
    LightData light[];
} lightData;

layout (set = 1, binding = 3) buffer _culledLights
{
    uint data[];
} culledLights;

layout (set = 1, binding = 4) buffer _gridLightData
{
    GridLight data[];
} gridBuffer;


// texture uniforms
layout (set = 2, binding = 0) uniform sampler2D diffTex;
layout (set = 3, binding = 0) uniform sampler2D normTex;
layout (set = 4, binding = 0) uniform sampler2D maskTex;

vec4 lightAmbient = vec4(0.2, 0.2, 0.2, 1.0);


void main()
{
#if HAS_COLOR_MASK == 1
    float mask = texture(maskTex, VertUV).r;
    if (mask.r < 0.5)
        discard;
#endif // HAS_COLOR_MASK == 1

    color = lightAmbient;

    uvec2 gridCoords = uvec2(gl_FragCoord.x / lightParams.pixelsPerViewFrustum,
                             gl_FragCoord.y / lightParams.pixelsPerViewFrustum);
    uint gridID = gridCoords.y * (lightParams.viewport.x / lightParams.pixelsPerViewFrustum)
                + gridCoords.x;

    #if HAS_NORMAL == 1
        mat3 TBN = transpose(mat3(VertTang, VertBitang, VertNorm));
    #endif // HAS_NORMAL == 1

    for (uint i = 0; i < gridBuffer.data[gridID].count; ++i)
    {
        uint lightOffset = gridBuffer.data[gridID].offset + i;
        LightData curLight = lightData.light[culledLights.data[lightOffset]];
        vec3 lightDir = curLight.pos.xyz - VertPosWorld;
        float distance = length(lightDir);

        #if HAS_NORMAL == 1
            lightDir = TBN * normalize(lightDir);
            vec3 texNorm = normalize(texture(normTex, VertUV).rgb * 2.0 - 1.0);
            texNorm.y = -texNorm.y;
            float coeff = dot(lightDir, texNorm);
        #else // HAS_NORMAL == 1
            lightDir = normalize(lightDir);
            float coeff = dot(lightDir, VertNorm);
        #endif // HAS_NORMAL == 1

        if (coeff < 0.0f) coeff = 0.0f;
        float att = (1.0f / (1.0f + 1.0f * distance * distance));

        if (distance < curLight.range)
            att *= (curLight.range - distance) / curLight.range;
        else
            att = 0;

        color += vec4(curLight.diffuse * coeff * att, 1.0f);
    }

    color *= materialParams.color;

#if HAS_TEXTURE == 1
    color *= texture(diffTex, VertUV);
#endif // HAS_TEXTURE == 1
}
