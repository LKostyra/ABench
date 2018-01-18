#pragma once

#include "Component.hpp"
#include "Common/Common.hpp"
#include "Math/Vector.hpp"


namespace ABench {
namespace Scene {

// workaround to allow easy storage of Light data
// this way we will be able to easily pass all the params to shaders
struct LightData
{
    Math::Vector4 position;
    Math::Vector4 diffuseIntensity;

    LightData()
        : position(0.0f, 0.0f, 0.0f, 1.0f)
        , diffuseIntensity(1.0f)
    {
    }
};

class Light: public Component
{
    friend class Scene;

    LightData mLightData;

public:
    Light(const std::string& name);
    ~Light();

    ABENCH_INLINE void SetPosition(float x, float y, float z)
    {
        SetPosition(Math::Vector4(x, y, z, 1.0f));
    }

    ABENCH_INLINE void SetPosition(const Math::Vector4& position)
    {
        mLightData.position = position;
    }

    ABENCH_INLINE void SetDiffuseIntensity(const Math::Vector4& intensity)
    {
        mLightData.diffuseIntensity = intensity;
    }

    ABENCH_INLINE const Math::Vector4& GetPosition() const
    {
        return mLightData.position;
    }

    ABENCH_INLINE const Math::Vector4& GetDiffuseIntensity() const
    {
        return mLightData.diffuseIntensity;
    }

    ABENCH_INLINE const LightData* GetData() const
    {
        return &mLightData;
    }

    ABENCH_INLINE ComponentType GetType() const override
    {
        return ComponentType::Light;
    }
};

} // namespace Scene
} // namespace ABench
