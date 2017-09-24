// temporarily changed from block because glslang has a bug
layout (location = 0) in vec3 VertNorm;
layout (location = 1) in vec2 VertUV;
layout (location = 2) in vec3 VertPosNoProj;


layout (location = 0) out vec4 color;


layout (set = 1, binding = 0) uniform sampler2D diffTex;
layout (set = 2, binding = 0) uniform sampler2D normTex;

layout (set = 3, binding = 0) uniform lightcb
{
    vec4 pos;
    vec4 diffuse;
} lightCBuffer;

vec4 lightAmbient = vec4(0.05, 0.05, 0.05, 1.0);

void main()
{
    color = lightAmbient;

    vec3 lightDir = lightCBuffer.pos.xyz - VertPosNoProj;
    float distance = length(lightDir);
    lightDir = normalize(lightDir);
    float coeff = dot(lightDir, VertNorm);

    color += coeff * lightCBuffer.diffuse * (5.0 / (distance * distance));

    #if HAS_TEXTURE == 1
        #if HAS_NORMAL == 1
            color *= texture(normTex, VertUV);
        #else // HAS_NORMAL == 1
            color *= texture(diffTex, VertUV);
        #endif // HAS_NORMAL == 1
    #else // HAS_TEXTURE == 1
        color *= vec4(0.3, 0.6, 0.9, 1.0);
    #endif // HAS_TEXTURE == 1
}
