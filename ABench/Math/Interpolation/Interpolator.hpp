#pragma once

#include "../Vector.hpp"

namespace ABench {
namespace Math {

using InterpolatorPoints = std::vector<Vector3>;

class Interpolator
{
protected:
    InterpolatorPoints mPoints;
    bool mOutOfRange;

    virtual Vector3 InterpolatePoints(const Vector3& p1, const Vector3& p2, float factor) = 0;

public:
    Interpolator();

    void Add(const Vector3& point);
    void Add(const InterpolatorPoints& points);
    void Clear();
    Vector3 Interpolate(float factor);

    bool OutOfRange() const
    {
        return mOutOfRange;
    }
};

} // namespace Math
} // namespace ABench
