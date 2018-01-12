#pragma once

#include "Common/Common.hpp"
#include "Math/Matrix.hpp"

#include "Component.hpp"


namespace ABench {
namespace Scene {

class Object final
{
    Math::Matrix mTransform;
    Math::Vector4 mPosition;
    Math::Vector4 mScale;
    Component* mComponent;
    mutable bool mToRender;

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
        SetPosition(Math::Vector4(x, y, z, 1.0f));
    }

    ABENCH_INLINE void SetPosition(const Math::Vector4& position)
    {
        mPosition = position;
        UpdateTransform();
    }

    ABENCH_INLINE void SetScale(float scaleX, float scaleY, float scaleZ)
    {
        SetScale(Math::Vector4(scaleX, scaleY, scaleZ, 1.0f));
    }

    ABENCH_INLINE void SetScale(const Math::Vector4& scale)
    {
        mScale = scale;
        UpdateTransform();
    }

    ABENCH_INLINE void SetToRender(bool toRender) const
    {
        mToRender = toRender;
    }

    ABENCH_INLINE Component* GetComponent() const
    {
        return mComponent;
    }

    ABENCH_INLINE const Math::Vector4& GetPosition() const
    {
        return mPosition;
    }

    ABENCH_INLINE const Math::Matrix& GetTransform() const
    {
        return mTransform;
    }

    ABENCH_INLINE bool ToRender() const
    {
        return mToRender;
    }
};

} // namespace Scene
} // namespace ABench
