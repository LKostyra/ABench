#include "PCH.hpp"
#include "Mesh.hpp"

#include "Common/Common.hpp"
#include "Common/Logger.hpp"

using namespace ABench::Renderer;


namespace ABench {
namespace Scene {

Mesh::Mesh()
{
}

Mesh::~Mesh()
{
}

bool Mesh::InitBuffers(const std::vector<Vertex>& vertices, int* indices, int indexCount)
{
    BufferDesc vbDesc;
    vbDesc.data = vertices.data();
    vbDesc.dataSize = vertices.size() * sizeof(Vertex);
    vbDesc.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
    vbDesc.type = BufferType::Static;
    if (!mVertexBuffer.Init(vbDesc))
        return false;

    BufferDesc ibDesc;
    ibDesc.data = indices;
    ibDesc.dataSize = indexCount * sizeof(uint32_t);
    ibDesc.usage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
    ibDesc.type = BufferType::Static;
    if (!mIndexBuffer.Init(ibDesc))
        return false;

    mIndexCount = indexCount;

    return true;
}

bool Mesh::InitFromFBX(FbxMesh* mesh)
{
    if (!mesh->GenerateNormals(true, true))
    {
        LOGE("Failed to generate normals for mesh");
        return false;
    }

    // find normal layer
    FbxLayerElementNormal* normals = nullptr;;
    for (int l = 0; l < mesh->GetLayerCount(); ++l)
    {
        normals = mesh->GetLayer(l)->GetNormals();
        if (normals)
            break;
    }

    if (!normals)
    {
        LOGE("Mesh has no normals generated!");
        return false;
    }

    if (normals->GetMappingMode() != FbxLayerElement::eByPolygonVertex)
    {
        LOGI("Normals were generated not by control point - recreating");
        if (!mesh->GenerateNormals(true, true))
        {
            LOGE("Failed to regenerate normals by control point");
            return nullptr;
        }

        for (int l = 0; l < mesh->GetLayerCount(); ++l)
        {
            normals = mesh->GetLayer(l)->GetNormals();
            if (normals)
                break;
        }

        if (!normals)
        {
            LOGE("Mesh has no normals generated!");
            return nullptr;
        }
    }

    std::vector<Vertex> vertices;
    Vertex vert;
    memset(&vert, 0, sizeof(vert));
    for (int p = 0; p < mesh->GetControlPointsCount(); ++p)
    {
        vert.pos[0] = static_cast<float>(mesh->GetControlPoints()[p].Buffer()[0]);
        vert.pos[1] = static_cast<float>(mesh->GetControlPoints()[p].Buffer()[1]);
        vert.pos[2] = static_cast<float>(mesh->GetControlPoints()[p].Buffer()[2]);
        vert.norm[0] = static_cast<float>(normals->GetDirectArray()[p].Buffer()[0]);
        vert.norm[1] = static_cast<float>(normals->GetDirectArray()[p].Buffer()[1]);
        vert.norm[2] = static_cast<float>(normals->GetDirectArray()[p].Buffer()[2]);
        vertices.push_back(vert);
    }

    return InitBuffers(vertices, mesh->GetPolygonVertices(), mesh->GetPolygonVertexCount());
}

bool Mesh::InitDefault()
{
    std::vector<Vertex> vertices
    {
        { -0.5f,-0.5f, 0.5f, 1.0f, 0.0f, 0.0f }, // 0        7----6
        {  0.5f,-0.5f, 0.5f, 0.0f, 1.0f, 0.0f }, // 1      3----2 |
        {  0.5f, 0.5f, 0.5f, 1.0f, 1.0f, 0.0f }, // 2      | 4--|-5
        { -0.5f, 0.5f, 0.5f, 0.0f, 0.0f, 1.0f }, // 3      0----1

        { -0.5f,-0.5f,-0.5f, 1.0f, 0.0f, 0.0f }, // 4
        {  0.5f,-0.5f,-0.5f, 0.0f, 1.0f, 0.0f }, // 5
        {  0.5f, 0.5f,-0.5f, 1.0f, 1.0f, 0.0f }, // 6
        { -0.5f, 0.5f,-0.5f, 0.0f, 0.0f, 1.0f }, // 7
    };

    std::vector<int> indices
    {
        0, 1, 2, 0, 2, 3, // front
        3, 2, 6, 3, 6, 7, // top
        7, 6, 5, 7, 5, 4, // back
        4, 5, 1, 4, 1, 0, // bottom
        1, 5, 6, 1, 6, 2, // right
        4, 0, 3, 4, 3, 7, // left
    };

    return InitBuffers(vertices, indices.data(), static_cast<int>(indices.size()));
}

bool Mesh::Init(FbxMesh* mesh)
{
    if (mesh)
        return InitFromFBX(mesh);
    else
        return InitDefault();
}

} // namespace Scene
} // namespace ABench
