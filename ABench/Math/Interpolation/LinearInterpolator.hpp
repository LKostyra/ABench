#pragma once

#include "Interpolator.hpp"


namespace ABench {
namespace Math {

class LinearInterpolator: public Interpolator
{
    float lerp(float p1, float p2, float factor);
    Vector3 InterpolatePoints(const Vector3& p1, const Vector3& p2, float factor) override;
};

} // namespace Math
} // namespace ABench