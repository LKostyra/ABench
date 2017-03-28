#pragma once

#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"


namespace ABench {
namespace Scene {

enum class CameraMode: unsigned char
{
    FREEROAM = 0,
    TRACED
};

struct CameraUpdateDesc
{
    Math::Vector pos;
    Math::Vector at;
    Math::Vector up;
};

struct CameraDesc
{
    CameraUpdateDesc view;

    float fov;
    float aspect;
    float nearZ;
    float farZ;
};

class Camera final
{
    Math::Vector mPosition;
    Math::Vector mAtPosition;
    Math::Vector mUpVector;

    Math::Matrix mView;
    Math::Matrix mProjection;

public:
    Camera();
    ~Camera();

    bool Init(const CameraDesc& desc);
    void Update(const CameraUpdateDesc& desc);

    __forceinline const Math::Vector& GetPosition() const
    {
        return mPosition;
    }

    __forceinline const Math::Vector& GetAtPosition() const
    {
        return mAtPosition;
    }

    __forceinline const Math::Vector& GetUpVector() const
    {
        return mUpVector;
    }

    __forceinline const Math::Matrix& GetView() const
    {
        return mView;
    }

    __forceinline const Math::Matrix& GetProjection() const
    {
        return mProjection;
    }
};

} // namespace Scene
} // namespace ABench
