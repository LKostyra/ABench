#pragma once

#include "Object.hpp"

#include "Renderer/Buffer.hpp"


namespace ABench {
namespace Scene {

class Mesh final
{
    ABench::Math::Matrix mWorldMatrix;
    ABench::Renderer::Buffer mVertexBuffer;
    ABench::Renderer::Buffer mIndexBuffer;

public:
    Mesh();
    ~Mesh();

    bool Init(ABench::Renderer::Device* devicePtr, const std::string& path);

    void SetPosition(float x, float y, float z);

    ABENCH_INLINE const ABench::Renderer::Buffer* GetVertexBuffer() const
    {
        return &mVertexBuffer;
    }

    ABENCH_INLINE const ABench::Renderer::Buffer* GetIndexBuffer() const
    {
        return &mIndexBuffer;
    }

    ABENCH_INLINE const ABench::Math::Matrix* GetWorldMatrix() const
    {
        return &mWorldMatrix;
    }
};

} // namespace Scene
} // namespace ABench