#pragma once

#include "Component.hpp"
#include "Common/Common.hpp"
#include "Math/Vector.hpp"


namespace ABench {
namespace Scene {

class Light: public Component
{
    Math::Vector4 mDiffuseIntensity;
    std::string mName; // TODO this should be in base class

public:
    Light(const std::string& name);
    ~Light();

    ABENCH_INLINE const Math::Vector4& GetDiffuseIntensity() const
    {
        return mDiffuseIntensity;
    }

    ABENCH_INLINE void SetDiffuseIntensity(const Math::Vector4& intensity)
    {
        mDiffuseIntensity = intensity;
    }

    ABENCH_INLINE ComponentType GetType() const
    {
        return ComponentType::Light;
    }
};

} // namespace Scene
} // namespace ABench
