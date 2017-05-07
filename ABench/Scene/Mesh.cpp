#include "PCH.hpp"
#include "Mesh.hpp"

#include "Common/Common.hpp"

using namespace ABench::Renderer;


namespace ABench {
namespace Scene {

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::Init(ABench::Renderer::Device* devicePtr, const std::string& path)
{
    UNUSED(path);

    float vertices[] =
    {
        -0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // 0        7----6
         0.5f,-0.5f, 0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // 1      3----2 |
         0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // 2      | 4--|-5
        -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // 3      0----1

        -0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f, 1.0f, // 4
         0.5f,-0.5f,-0.5f, 0.0f, 1.0f, 0.0f, 1.0f, // 5
         0.5f, 0.5f,-0.5f, 1.0f, 1.0f, 0.0f, 1.0f, // 6
        -0.5f, 0.5f,-0.5f, 0.0f, 0.0f, 1.0f, 1.0f, // 7
    };

    uint32_t indices[] =
    {
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // top
        7, 6, 5, 7, 5, 4, // back
        4, 5, 1, 4, 1, 0, // bottom
        1, 5, 6, 1, 6, 2, // right
        4, 0, 3, 4, 3, 7, // left
    };

    BufferDesc vbDesc;
    vbDesc.devicePtr = devicePtr;
    vbDesc.data = vertices;
    vbDesc.dataSize = sizeof(vertices);
    vbDesc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vbDesc.type = BufferType::Static;
    if (!mVertexBuffer.Init(vbDesc))
        return false;

    BufferDesc ibDesc;
    ibDesc.devicePtr = devicePtr;
    ibDesc.data = indices;
    ibDesc.dataSize = sizeof(indices);
    ibDesc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    ibDesc.type = BufferType::Static;
    if (!mIndexBuffer.Init(ibDesc))
        return false;

    return true;
}

} // namespace Scene
} // namespace ABench
