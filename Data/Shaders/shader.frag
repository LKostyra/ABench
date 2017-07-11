// temporarily changed from block because glslang has a bug
layout (location = 0) in vec3 VertNorm;
layout (location = 1) in vec2 VertUV;


layout (location = 0) out vec4 color;


layout (set = 1, binding = 0) uniform sampler2D tex;


void main()
{
    color = texture(tex, VertUV);
}
