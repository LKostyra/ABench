#pragma once

#include "Object.hpp"
#include "Component.hpp"

#include "Renderer/Buffer.hpp"


namespace ABench {
namespace Scene {

struct Vertex
{
    float pos[3];
    float norm[3];
};

class Mesh final: public Component
{
    ABench::Renderer::Buffer mVertexBuffer;
    ABench::Renderer::Buffer mIndexBuffer;
    uint32_t mIndexCount;

    bool InitBuffers(const std::vector<Vertex>& vertices, int* indices, int indexCount);
    bool InitFromFBX(FbxMesh* mesh);
    bool InitDefault();

public:
    Mesh();
    ~Mesh();

    bool Init(FbxMesh* mesh = nullptr);

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

    ComponentType GetType() const
    {
        return ComponentType::Mesh;
    }
};

} // namespace Scene
} // namespace ABench