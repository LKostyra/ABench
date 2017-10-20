#pragma once

#include "Object.hpp"
#include "Component.hpp"
#include "Material.hpp"

#include "Renderer/Buffer.hpp"


namespace ABench {
namespace Scene {

struct Vertex
{
    float pos[3];
    float norm[3];
    float uv[2];
};

class Mesh final
{
    ABench::Renderer::Buffer mVertexBuffer;
    ABench::Renderer::Buffer mIndexBuffer;
    uint32_t mPointCount;
    Material* mMaterial;
    bool mByIndices;

    bool InitBuffers(const std::vector<Vertex>& vertices, int* indices, int indexCount);
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
        return &mVertexBuffer;
    }

    ABENCH_INLINE const ABench::Renderer::Buffer* GetIndexBuffer() const
    {
        return &mIndexBuffer;
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
