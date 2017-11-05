#pragma once

#include <iostream>

namespace ABench {
namespace Math {

class Vector3;
class Vector4;

class Vector3
{
    float f[3];

public:
    friend class Vector4;

    float* Data();

    float Length() const;
    void Normalize();

    Vector3();
    Vector3(float a);
    Vector3(float a, float b, float c);
    Vector3(const Vector4& v);
    ~Vector3();

    Vector3(const Vector3& other);
    Vector3(Vector3&& other);
    Vector3& operator= (const Vector3& other);
    Vector3& operator= (Vector3&& other);

    // Access operator
    float operator[](int index) const;

    // Addition
    Vector3& operator+=(const Vector3& other);
    Vector3& operator+=(float value);
    const Vector3 operator+(const Vector3& other) const;
    const Vector3 operator+(float value) const;

    // Subtraction
    Vector3& operator-=(const Vector3& other);
    Vector3& operator-=(float value);
    const Vector3 operator-(const Vector3& other) const;
    const Vector3 operator-(float value) const;

    // Multiplication
    Vector3& operator*=(const Vector3& other);
    Vector3& operator*=(float value);
    const Vector3 operator*(const Vector3& other) const;
    const Vector3 operator*(float value) const;

    // Division
    Vector3& operator/=(const Vector3& other);
    Vector3& operator/=(float value);
    const Vector3 operator/(const Vector3& other) const;
    const Vector3 operator/(float value) const;

    // Power
    Vector3& operator^(float value);

    // Products
    float Dot(const Vector3& other);
    Vector3 Cross(const Vector3& other) const;

    // Comparison
    bool operator==(const Vector3& other) const;
    bool operator<(const Vector3& other) const;
    bool operator>(const Vector3& other) const;
    bool operator<=(const Vector3& other) const;
    bool operator>=(const Vector3& other) const;

    // Friendships
    friend std::ostream& operator<<(std::ostream& os, const Vector3& v);
};

class Vector4
{
    float f[4];

public:
    friend class Vector3;

    float* Data();

    float Length() const;
    void Normalize();

    Vector4();
    Vector4(float a);
    Vector4(float a, float b, float c, float d);
    Vector4(const Vector3& v, float w);
    ~Vector4();

    Vector4(const Vector4& other);
    Vector4(Vector4&& other);
    Vector4& operator= (const Vector4& other);
    Vector4& operator= (Vector4&& other);

    // Access operator
    float operator[](int index) const;

    // Addition
    Vector4& operator+=(const Vector4& other);
    Vector4& operator+=(float value);
    const Vector4 operator+(const Vector4& other) const;
    const Vector4 operator+(float value) const;

    // Subtraction
    Vector4& operator-=(const Vector4& other);
    Vector4& operator-=(float value);
    const Vector4 operator-(const Vector4& other) const;
    const Vector4 operator-(float value) const;

    // Multiplication
    Vector4& operator*=(const Vector4& other);
    Vector4& operator*=(float value);
    const Vector4 operator*(const Vector4& other) const;
    const Vector4 operator*(float value) const;

    // Division
    Vector4& operator/=(const Vector4& other);
    Vector4& operator/=(float value);
    const Vector4 operator/(const Vector4& other) const;
    const Vector4 operator/(float value) const;

    // Power
    Vector4& operator^(float value);

    // Products
    float Dot(const Vector4& other);
    Vector4 Cross(const Vector4& other) const;

    // Comparison
    bool operator==(const Vector4& other) const;
    bool operator<(const Vector4& other) const;
    bool operator>(const Vector4& other) const;
    bool operator<=(const Vector4& other) const;
    bool operator>=(const Vector4& other) const;

    // Friendships
    friend std::ostream& operator<<(std::ostream& os, const Vector4& v);
};

std::ostream& operator<<(std::ostream& os, const Vector3& v);
std::ostream& operator<<(std::ostream& os, const Vector4& v);

} // namespace Math
} // namespace ABench
