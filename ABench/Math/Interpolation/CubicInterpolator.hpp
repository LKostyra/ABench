#pragma once

#include "Interpolator.hpp"


namespace ABench {
namespace Math {

class CubicInterpolator: public Interpolator
{
    Vector3 InterpolatePoints(const Vector3& p1, const Vector3& p2, float factor) override;
};

} // namespace Math
} // namespace ABench
