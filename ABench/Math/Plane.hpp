#pragma once

#include "Vector.hpp"
#include "Common/Common.hpp"


namespace ABench {
namespace Math {

class Plane
{
    friend class Frustum;

    Vector3 n;
    float d;

public:
    Plane();
    Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2);

    ABENCH_INLINE const Vector3& GetNormal() const
    {
        return n;
    }

    friend std::ostream& operator<<(std::ostream& o, const Plane& plane);
};

std::ostream& operator<<(std::ostream& o, const Plane& plane);

} // namespace Math
} // namespace ABench
