#pragma once

#include "Math/Matrix.hpp"
#include "Renderer/CommandBuffer.hpp"

#include "Component.hpp"


namespace ABench {
namespace Scene {

class Object final
{
    Math::Matrix mTransform;
    Component* mComponent;

public:
    Object();
    ~Object();

    ABENCH_INLINE void SetComponent(Component* component)
    {
        mComponent = component;
    }

    ABENCH_INLINE void SetPosition(float x, float y, float z)
    {
        mTransform = Math::CreateTranslationMatrix(Math::Vector(x, y, z, 1.0f));
    }

    ABENCH_INLINE Component* GetComponent() const
    {
        return mComponent;
    }

    ABENCH_INLINE const Math::Matrix& GetTransform() const
    {
        return mTransform;
    }
};

} // namespace Scene
} // namespace ABench
