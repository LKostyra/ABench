layout (location = 0) in vec3 InPos;
layout (location = 1) in vec2 InUV;
layout (location = 2) in vec4 InInstancePos;
layout (location = 3) in vec3 InInstanceColor;
layout (location = 4) in float InInstanceLifeTimer;
layout (location = 5) in vec3 InInstanceVel;

layout (location = 0) out vec3 VertColor;
layout (location = 1) out float VertLifeTimer;
layout (location = 2) out vec2 VertUV;


layout (set = 0, binding = 0) uniform cb
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
    mat4 worldMatrix = mat4(vec4(1.0f, 0.0f, 0.0f, 0.0f),
                            vec4(0.0f, 1.0f, 0.0f, 0.0f),
                            vec4(0.0f, 0.0f, 1.0f, 0.0f),
                            InInstancePos);

    vec3 cameraRight = vec3(CBuffer.viewMatrix[0][0], CBuffer.viewMatrix[1][0], CBuffer.viewMatrix[2][0]);
    vec3 cameraUp = vec3(CBuffer.viewMatrix[0][1], CBuffer.viewMatrix[1][1], CBuffer.viewMatrix[2][1]);

    vec3 vertPos = InInstancePos.xyz + cameraRight * InPos.x - InInstanceVel * InPos.y;
    mat4 viewProj = CBuffer.projMatrix * CBuffer.viewMatrix;

    VertColor = InInstanceColor;
    VertLifeTimer = InInstanceLifeTimer;
    VertUV = InUV;
    gl_Position = viewProj * vec4(vertPos, 1.0);
}
