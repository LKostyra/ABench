#include "PCH.hpp"
#include "Interpolator.hpp"


namespace ABench {
namespace Math {

Interpolator::Interpolator()
{
}

void Interpolator::Add(const Vector3& point)
{
    mPoints.emplace_back(point);
}

void Interpolator::Add(const InterpolatorPoints& points)
{
    mPoints.insert(mPoints.end(), points.begin(), points.end());
}

void Interpolator::Clear()
{
    mPoints.clear();
}

Vector3 Interpolator::Interpolate(float factor)
{
    if (mPoints.size() == 0)
        return Vector3(0.0f);
    if (mPoints.size() == 1)
        return mPoints[0];

    if (factor <= 0.0f)
        return mPoints[0];
    else if (factor >= 1.0f)
        return mPoints.back();

    // determine where we are in our point list
    factor *= mPoints.size() - 1;
    uint32_t point0 = static_cast<uint32_t>(factor);
    factor -= point0;

    // call backend
    return InterpolatePoints(mPoints[point0], mPoints[point0 + 1], factor);
}

} // namespace Math
} // namespace ABench
