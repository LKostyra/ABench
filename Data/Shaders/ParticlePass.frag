layout (early_fragment_tests) in;

layout (location = 0) in vec3 VertColor;
layout (location = 1) in float VertLifeTimer;
layout (location = 2) in vec2 VertUV;

layout (location = 0) out vec4 color;

layout (set = 1, binding = 0) uniform sampler2D particleTex;


void main()
{
    color = texture(particleTex, VertUV);
    color *= vec4(VertColor, 1.0f);
    if (VertLifeTimer < 1.0f)
        color.w *= VertLifeTimer;
}
