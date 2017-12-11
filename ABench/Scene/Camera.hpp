#pragma once

#include "Math/Vector.hpp"
#include "Math/Matrix.hpp"
#include "Common/Common.hpp"
#include "Renderer/HighLevel/ResourceManager.hpp"


namespace ABench {
namespace Scene {

enum class CameraMode: unsigned char
{
    FREEROAM = 0,
    TRACED
};

struct CameraUpdateDesc
{
    Math::Vector4 pos;
    Math::Vector4 at;
    Math::Vector4 up;
};

struct CameraDesc
{
    CameraUpdateDesc view;

    float fov;
    float nearZ;
    float farZ;
    uint32_t windowWidth;
    uint32_t windowHeight;
};

class Camera final
{
    Math::Vector4 mPosition;
    Math::Vector4 mAtPosition;
    Math::Vector4 mUpVector;

    Math::Matrix mView;
    Math::Matrix mProjection;

    Renderer::BufferPtr mGridFrustums;

public:
    Camera();
    ~Camera();

    bool Init(const CameraDesc& desc);
    void Update(const CameraUpdateDesc& desc);

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

    ABENCH_INLINE const Math::Matrix& GetProjection() const
    {
        return mProjection;
    }

    ABENCH_INLINE const Renderer::Buffer* GetGridFrustums() const
    {
        return mGridFrustums.get();
    }
};

} // namespace Scene
} // namespace ABench
