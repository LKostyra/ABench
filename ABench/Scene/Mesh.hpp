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

class Mesh final: public Component
{
    ABench::Renderer::Buffer mVertexBuffer;
    ABench::Renderer::Buffer mIndexBuffer;
    uint32_t mPointCount;
    Material* mMaterial;
    std::string mName;
    bool mByIndices;

    bool InitBuffers(const std::vector<Vertex>& vertices, int* indices, int indexCount);
    bool InitFromFBX(FbxMesh* mesh);
    bool InitDefault();

public:
    Mesh(const std::string& name);
    ~Mesh();

    bool Init(FbxMesh* mesh = nullptr);

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

    ABENCH_INLINE const std::string& GetName() const
    {
        return mName;
    }

    ABENCH_INLINE bool ByIndices() const
    {
        return mByIndices;
    }

    ComponentType GetType() const override
    {
        return ComponentType::Mesh;
    }
};

} // namespace Scene
} // namespace ABench