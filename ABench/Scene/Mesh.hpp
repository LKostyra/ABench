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
    uint32_t mIndexCount;
    Material* mMaterial;
    std::string mName;

    bool InitBuffers(const std::vector<Vertex>& vertices, int* indices, int indexCount);
    bool InitFromFBX(FbxMesh* mesh);
    bool InitDefault();

public:
    Mesh();
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

    ABENCH_INLINE const uint32_t GetIndexCount() const
    {
        return mIndexCount;
    }

    ABENCH_INLINE const Material* GetMaterial() const
    {
        return mMaterial;
    }

    ABENCH_INLINE const std::string& GetName() const
    {
        return mName;
    }

    ComponentType GetType() const override
    {
        return ComponentType::Mesh;
    }
};

} // namespace Scene
} // namespace ABench