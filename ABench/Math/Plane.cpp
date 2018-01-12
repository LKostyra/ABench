#include "PCH.hpp"
#include "Plane.hpp"


namespace ABench {
namespace Math {

Plane::Plane()
    : n()
    , d(0.0f)
{
}

Plane::Plane(const Vector3& p0, const Vector3& p1, const Vector3& p2)
{
    Vector3 v0 = p1 - p0;
    Vector3 v2 = p2 - p0;

    n = v0.Cross(v2);
    n.Normalize();
    d = n.Dot(p0);
}

std::ostream& operator<<(std::ostream& o, const Plane& plane)
{
    o << "[" << plane.n[0] << ", " << plane.n[1] << ", " << plane.n[2] << "], " << plane.d;
    return o;
}

} // namespace Math
} // namespace ABench
