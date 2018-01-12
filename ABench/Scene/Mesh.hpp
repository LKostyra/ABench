#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Material.hpp"

#include "Math/AABB.hpp"
#include "Renderer/HighLevel/ResourceManager.hpp"


namespace ABench {
namespace Scene {

struct Vertex
{
    float pos[3];
};

struct VertexParams
{
    float norm[3];
    float uv[2];
    float tang[3];
};

class Mesh final
{
    ABench::Renderer::BufferPtr mVertexBuffer;
    ABench::Renderer::BufferPtr mVertexParamsBuffer;
    ABench::Renderer::BufferPtr mIndexBuffer;
    uint32_t mPointCount;
    Material* mMaterial;
    bool mByIndices;

    bool HasNormalMap(FbxMesh* mesh, int materialIndex);
    bool InitBuffers(const std::vector<Vertex>& vertices,
                     const std::vector<VertexParams>& vertexParams,
                     int* indices, int indexCount);
    bool InitFromFBX(FbxMesh* mesh, int materialIndex);
    bool InitDefault();

public:
    Mesh();
    ~Mesh();

    bool Init(FbxMesh* mesh, uint32_t materialIndex);

    ABENCH_INLINE void SetMaterial(Material* mat)
    {
        mMaterial = mat;
    }

    ABENCH_INLINE const ABench::Renderer::Buffer* GetVertexBuffer() const
    {
        return mVertexBuffer.get();
    }

    ABENCH_INLINE const ABench::Renderer::Buffer* GetVertexParamsBuffer() const
    {
        return mVertexParamsBuffer.get();
    }

    ABENCH_INLINE const ABench::Renderer::Buffer* GetIndexBuffer() const
    {
        return mIndexBuffer.get();
    }

    ABENCH_INLINE const uint32_t GetPointCount() const
    {
        return mPointCount;
    }

    ABENCH_INLINE const Material* GetMaterial() const
    {
        return mMaterial;
    }

    ABENCH_INLINE bool ByIndices() const
    {
        return mByIndices;
    }
};

} // namespace Scene
} // namespace ABench
