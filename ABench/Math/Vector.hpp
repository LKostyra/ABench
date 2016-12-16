#pragma once

#include <iostream>

namespace ABench {
namespace Math {

class Vector
{
public:
    const float* Data() const;

    float Length() const;
    void Normalize();

    Vector();
    Vector(float a);
    Vector(float a, float b, float c, float d);
    ~Vector();

    Vector(const Vector& other);
    Vector(Vector&& other);
    Vector& operator= (const Vector& other);
    Vector& operator= (Vector&& other);

    // Access operator
    float operator[](int index) const;

    // Addition
    Vector& operator+=(const Vector& other);
    Vector& operator+=(float value);
    const Vector operator+(const Vector& other) const;
    const Vector operator+(float value) const;

    // Subtraction
    Vector& operator-=(const Vector& other);
    Vector& operator-=(float value);
    const Vector operator-(const Vector& other) const;
    const Vector operator-(float value) const;

    // Multiplication
    Vector& operator*=(const Vector& other);
    Vector& operator*=(float value);
    const Vector operator*(const Vector& other) const;
    const Vector operator*(float value) const;

    // Division
    Vector& operator/=(const Vector& other);
    Vector& operator/=(float value);
    const Vector operator/(const Vector& other) const;
    const Vector operator/(float value) const;

    // Power
    Vector& operator^(float value);

    // Products
    float Dot(const Vector& other);
    Vector Cross(const Vector& other) const;

    // Comparison
    bool operator==(const Vector& other) const;
    bool operator<(const Vector& other) const;
    bool operator>(const Vector& other) const;
    bool operator<=(const Vector& other) const;
    bool operator>=(const Vector& other) const;

    // Friendships
    friend std::ostream& operator<<(std::ostream& os, const Vector& v);

private:
    float f[4];
};

struct Vector4
{
    Vector v[4];
};

std::ostream& operator<<(std::ostream& os, const Vector& v);

} // namespace Math
} // namespace ABench
