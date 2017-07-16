#pragma once

#include "Math/Matrix.hpp"
#include "Renderer/CommandBuffer.hpp"

#include "Component.hpp"


namespace ABench {
namespace Scene {

class Object final
{
    Math::Matrix mTransform;
    Math::Vector mPosition;
    Math::Vector mScale;
    Component* mComponent;

    ABENCH_INLINE void UpdateTransform()
    {
        mTransform = Math::CreateTranslationMatrix(mPosition) * Math::CreateScaleMatrix(mScale);
    }

public:
    Object();
    ~Object();

    ABENCH_INLINE void SetComponent(Component* component)
    {
        mComponent = component;
    }

    ABENCH_INLINE void SetPosition(float x, float y, float z)
    {
        SetPosition(Math::Vector(x, y, z, 1.0f));
    }

    ABENCH_INLINE void SetPosition(const Math::Vector& position)
    {
        mPosition = position;
        UpdateTransform();
    }

    ABENCH_INLINE void SetScale(float scaleX, float scaleY, float scaleZ)
    {
        SetScale(Math::Vector(scaleX, scaleY, scaleZ, 1.0f));
    }

    ABENCH_INLINE void SetScale(const Math::Vector& scale)
    {
        mScale = scale;
        UpdateTransform();
    }

    ABENCH_INLINE Component* GetComponent() const
    {
        return mComponent;
    }

    ABENCH_INLINE const Math::Vector& GetPosition() const
    {
        return mPosition;
    }

    ABENCH_INLINE const Math::Matrix& GetTransform() const
    {
        return mTransform;
    }
};

} // namespace Scene
} // namespace ABench
