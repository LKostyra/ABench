#include "PCH.hpp"
#include "AABB.hpp"

namespace ABench {
namespace Math {

AABB::AABB()
{
}

AABB::AABB(Vector4 min, Vector4 max)
{
    mVert[AABBVert::MIN] = min;
    mVert[AABBVert::MAX] = max;
}

std::ostream& operator<<(std::ostream& o, const AABB& aabb)
{
    o << "min: " << aabb.mVert[aabb.AABBVert::MIN] << ", max: " << aabb.mVert[aabb.AABBVert::MAX];
    return o;
}

} // namespace Math
} // namespace ABench
