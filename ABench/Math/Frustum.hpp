#pragma once

#include "Plane.hpp"
#include "AABB.hpp"


namespace ABench {
namespace Math {

struct FrustumDesc
{
    float ratio;
    float fov;
    float nearZ;
    float farZ;

    FrustumDesc()
        : ratio(0.0f)
        , fov(0.0f)
        , nearZ(0.0f)
        , farZ(0.0f)
    {
    }
};

class Frustum
{
    enum PlaneSide
    {
        FRONT = 0,
        LEFT,
        TOP,
        RIGHT,
        BOTTOM,
        BACK,
        COUNT
    };

    Plane mPlanes[PlaneSide::COUNT];
    float mScreenRatio;
    float mHalfFovTan;
    float mNearZ;
    float mFarZ;

public:
    Frustum();
    ~Frustum();

    void Init(const FrustumDesc& desc);
    void Refresh(const Vector3& pos, const Vector3& at, const Vector3& up);
    bool Intersects(const AABB& aabb) const; // true if even partially inside frustum
};

} // namespace Math
} // namespace ABench
