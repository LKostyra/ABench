#include "PCH.hpp"
#include "LinearInterpolator.hpp"


namespace ABench {
namespace Math {

float LinearInterpolator::lerp(float p1, float p2, float factor)
{
    return (1.0f - factor) * p1 + factor * p2;
}

Vector3 LinearInterpolator::InterpolatePoints(const Vector3& p1, const Vector3& p2, float factor)
{
    return Vector3(lerp(p1[0], p2[0], factor),
                   lerp(p1[1], p2[1], factor),
                   lerp(p1[2], p2[2], factor));
}

} // namespace Math
} // namespace ABench