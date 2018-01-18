#include "PCH.hpp"
#include "Frustum.hpp"
#include "Common.hpp"

namespace ABench {
namespace Math {

Frustum::Frustum()
    : mScreenRatio(0.0f)
    , mHalfFovTan(0.0f)
    , mNearZ(0.0f)
    , mFarZ(0.0f)
{
}

Frustum::~Frustum()
{
}

void Frustum::Init(const FrustumDesc& desc)
{
    float angle = MATH_DEG_TO_RAD(desc.fov * 0.5f);
    mHalfFovTan = cosf(angle) / sinf(angle);
    mScreenRatio = desc.ratio;
    mNearZ = desc.nearZ;
    mFarZ = desc.farZ;
}

void Frustum::Refresh(const Vector3& pos, const Vector3& at, const Vector3& up)
{
    // camera's axes
    Vector3 cameraZ = at - pos;
    cameraZ.Normalize();
    Vector3 cameraX = up.Cross(cameraZ);
    cameraX.Normalize();
    Vector3 cameraY = cameraZ.Cross(cameraX);
    cameraY.Normalize();

    // near points
    Vector3 nearMid = pos + (cameraZ * mNearZ);
    float nearHalfH = mNearZ * mHalfFovTan;
    float nearHalfW = nearHalfH / mScreenRatio;
    Vector3 nearTopLeft = nearMid - (cameraX * nearHalfW) + (cameraY * nearHalfH);
    Vector3 nearTopRight = nearMid + (cameraX * nearHalfW) + (cameraY * nearHalfH);
    Vector3 nearBottomLeft = nearMid - (cameraX * nearHalfW) - (cameraY * nearHalfH);
    Vector3 nearBottomRight = nearMid + (cameraX * nearHalfW) - (cameraY * nearHalfH);

    // far points
    Vector3 farMid = pos + (cameraZ * mFarZ);
    float farHalfH = mFarZ * mHalfFovTan;
    float farHalfW = farHalfH / mScreenRatio;
    Vector3 farTopLeft = farMid - (cameraX * farHalfW) + (cameraY * farHalfH);
    Vector3 farTopRight = farMid + (cameraX * farHalfW) + (cameraY * farHalfH);
    Vector3 farBottomLeft = farMid - (cameraX * farHalfW) - (cameraY * farHalfH);
    Vector3 farBottomRight = farMid + (cameraX * farHalfW) - (cameraY * farHalfH);

    // calculated assuming LH coordinate system
    mPlanes[PlaneSide::FRONT] = Plane(nearBottomLeft, nearBottomRight, nearTopLeft);
    mPlanes[PlaneSide::LEFT] = Plane(nearBottomLeft, nearTopLeft, farBottomLeft);
    mPlanes[PlaneSide::TOP] = Plane(nearTopLeft, nearTopRight, farTopLeft);
    mPlanes[PlaneSide::RIGHT] = Plane(nearTopRight, nearBottomRight, farTopRight);
    mPlanes[PlaneSide::BOTTOM] = Plane(nearBottomRight, nearBottomLeft, farBottomRight);
    mPlanes[PlaneSide::BACK] = Plane(farTopLeft, farTopRight, farBottomLeft);
}

bool Frustum::Intersects(const AABB& aabb) const
{
    // check for every plane
    for (uint32_t i = 0; i < PlaneSide::COUNT; ++i)
    {
        // acquire p-vertex's index from Plane's normal
        uint32_t px = static_cast<uint32_t>(mPlanes[i].n[0] > 0.0f);
        uint32_t py = static_cast<uint32_t>(mPlanes[i].n[1] > 0.0f);
        uint32_t pz = static_cast<uint32_t>(mPlanes[i].n[2] > 0.0f);

        // calculate dot product of Plane's normal and our p-vertex
        Vector3 pvert = Vector3(aabb.mVert[px][0], aabb.mVert[py][1], aabb.mVert[pz][2]);
        float pdot = mPlanes[i].GetNormal().Dot(pvert);

        if (pdot < mPlanes[i].d)
        {
            return false;
        }
    }

    return true;
}

} // namespace Math
} // namespace ABench
