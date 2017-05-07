#pragma once

#include "Object.hpp"
#include "Component.hpp"

#include "Renderer/Buffer.hpp"


namespace ABench {
namespace Scene {

class Mesh final: public Component
{
    ABench::Renderer::Buffer mVertexBuffer;
    ABench::Renderer::Buffer mIndexBuffer;

public:
    Mesh();
    ~Mesh();

    bool Init(ABench::Renderer::Device* devicePtr, const std::string& inScenePath);

    ABENCH_INLINE const ABench::Renderer::Buffer* GetVertexBuffer() const
    {
        return &mVertexBuffer;
    }

    ABENCH_INLINE const ABench::Renderer::Buffer* GetIndexBuffer() const
    {
        return &mIndexBuffer;
    }

    ComponentType GetType() const
    {
        return ComponentType::Mesh;
    }
};

} // namespace Scene
} // namespace ABench