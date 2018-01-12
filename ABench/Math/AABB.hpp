#pragma once

#include "Vector.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Math {

class AABB
{
    friend class Frustum;

public:
    enum AABBVert
    {
        MIN = 0,
        MAX,
        COUNT
    };

private:
    Vector4 mVert[AABBVert::COUNT];

public:
    AABB();
    AABB(Vector4 min, Vector4 max);

    ABENCH_INLINE void SetVert(const Vector4& v, AABBVert type)
    {
        mVert[type] = v;
    }

    ABENCH_INLINE const Vector4& GetVert(AABBVert v) const
    {
        return mVert[v];
    }

    ABENCH_INLINE const Vector4& operator[](AABBVert v) const
    {
        return mVert[v];
    }

    friend std::ostream& operator<<(std::ostream& o, const AABB& aabb);
};

std::ostream& operator<<(std::ostream& o, const AABB& aabb);

} // namespace Math
} // namespace ABench
