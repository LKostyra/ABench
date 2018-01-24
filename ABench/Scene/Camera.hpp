#pragma once

#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Math/Frustum.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Scene {

enum class CameraMode: unsigned char
{
    FREEROAM = 0,
    TRACED
};

struct CameraDesc
{
    Math::Vector4 pos;
    Math::Vector4 at;
    Math::Vector4 up;
};

class Camera final
{
    Math::Vector4 mPosition;
    Math::Vector4 mAtPosition;
    Math::Vector4 mUpVector;
    Math::Matrix mView;

public:
    void Update(const CameraDesc& desc);

    ABENCH_INLINE const Math::Vector4& GetPosition() const
    {
        return mPosition;
    }

    ABENCH_INLINE const Math::Vector4& GetAtPosition() const
    {
        return mAtPosition;
    }

    ABENCH_INLINE const Math::Vector4& GetUpVector() const
    {
        return mUpVector;
    }

    ABENCH_INLINE const Math::Matrix& GetView() const
    {
        return mView;
    }
};

} // namespace Scene
} // namespace ABench
