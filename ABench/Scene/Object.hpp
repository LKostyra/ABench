#pragma once

#include "Common/Common.hpp"
#include "Math/Matrix.hpp"

#include "Component.hpp"


namespace ABench {
namespace Scene {

class Object final
{
    Component* mComponent;

public:
    Object();
    ~Object();

    ABENCH_INLINE void SetComponent(Component* component)
    {
        mComponent = component;
    }

    ABENCH_INLINE Component* GetComponent() const
    {
        return mComponent;
    }
};

} // namespace Scene
} // namespace ABench
