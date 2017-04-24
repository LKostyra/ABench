#pragma once

#include "Math/Matrix.hpp"
#include "Renderer/CommandBuffer.hpp"

namespace ABench {
namespace Scene {

class Object
{
    friend class Renderer;

protected:
    Math::Matrix mTransform;

    ABENCH_INLINE const Math::Matrix& GetTransform() const
    {
        return mTransform;
    }

public:
    Object();
    ~Object();

    void SetPosition(float x, float y, float z);
};

} // namespace Scene
} // namespace ABench
