#pragma once

#include "Math/Matrix.hpp"

namespace ABench {
namespace Scene {

class Object
{
    friend class Renderer;

protected:
    Math::Matrix mTransform;

    __forceinline const Math::Matrix& GetTransform() const
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
