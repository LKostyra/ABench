#include "PCH.hpp"
#include "Vector.hpp"

#include <algorithm>
#include <cstring>
#include <cmath>

namespace ABench {
namespace Math {

// VECTOR 3 //

Vector3::Vector3()
    : Vector3(0.0f)
{
}

Vector3::Vector3(float a)
    : f{a, a, a}
{
}

Vector3::Vector3(float a, float b, float c)
    : f{a, b, c}
{
}

Vector3::Vector3(const Vector4& v)
    : f{v.f[0], v.f[1], v.f[2]}
{
}

Vector3::~Vector3()
{
}

Vector3::Vector3(const Vector3& other)
{
    memcpy(f, other.f, sizeof(float) * 3);
}

Vector3::Vector3(Vector3&& other)
    : Vector3()
{
    std::swap(f, other.f);
}

float* Vector3::Data()
{
    return f;
}

float Vector3::Length() const
{
    Vector3 temp(*this);

    return sqrtf(temp.Dot(temp));
}

void Vector3::Normalize()
{
    float len = Length();

    if (len)
        *this = *this / len;
}

Vector3& Vector3::operator= (const Vector3& other)
{
    Vector3 tmp(other);
    *this = std::move(tmp);

    return *this;
}

Vector3& Vector3::operator= (Vector3&& other)
{
    std::swap(f, other.f);

    return *this;
}

// Access operator
float Vector3::operator[](int index) const
{
    return f[index];
}

// Addition
Vector3& Vector3::operator+=(const Vector3& other)
{
    for (int i = 0; i < 3; ++i)
        f[i] += other.f[i];

    return *this;
}

Vector3& Vector3::operator+=(float value)
{
    for (auto& i : f)
        i += value;

    return *this;
}

const Vector3 Vector3::operator+(const Vector3& other) const
{
    return Vector3(*this) += other;
}

const Vector3 Vector3::operator+(float value) const
{
    return Vector3(*this) += value;
}

// Subtraction
Vector3& Vector3::operator-=(const Vector3& other)
{
    for (int i = 0; i < 3; ++i)
        f[i] -= other.f[i];

    return *this;
}

Vector3& Vector3::operator-=(float value)
{
    for (auto& i : f)
        i -= value;

    return *this;
}

const Vector3 Vector3::operator-(const Vector3& other) const
{
    return Vector3(*this) -= other;
}

const Vector3 Vector3::operator-(float value) const
{
    return Vector3(*this) -= value;
}

// Multiplication
Vector3& Vector3::operator*=(const Vector3& other)
{
    for (int i = 0; i < 3; ++i)
        f[i] *= other.f[i];

    return *this;
}

Vector3& Vector3::operator*=(float value)
{
    for (auto& i : f)
        i *= value;

    return *this;
}

const Vector3 Vector3::operator*(const Vector3& other) const
{
    return Vector3(*this) *= other;
}

const Vector3 Vector3::operator*(float value) const
{
    return Vector3(*this) *= value;
}

// Division
Vector3& Vector3::operator/=(const Vector3& other)
{
    for (int i = 0; i < 3; ++i)
        f[i] /= other.f[i];

    return *this;
}

Vector3& Vector3::operator/=(float value)
{
    for (auto& i : f)
        i /= value;

    return *this;
}

const Vector3 Vector3::operator/(const Vector3& other) const
{
    return Vector3(*this) /= other;
}

const Vector3 Vector3::operator/(float value) const
{
    return Vector3(*this) /= value;
}

// Power
Vector3& Vector3::operator^(float value)
{
    for (auto& i : f)
        i = pow(i, value);

    return *this;
}

// Products
float Vector3::Dot(const Vector3& other)
{
    Vector3 tmp(*this);
    tmp = tmp * other;

    float result = 0;
    for (auto i : tmp.f)
        result += i;

    return result;
}

Vector3 Vector3::Cross(const Vector3& other) const
{
    Vector3 result(*this);

    // calculate cross according to xyzzy mnemonic
    result.f[0] = this->f[1] * other.f[2] - this->f[2] * other.f[1];
    result.f[1] = this->f[2] * other.f[0] - this->f[0] * other.f[2];
    result.f[2] = this->f[0] * other.f[1] - this->f[1] * other.f[0];

    return result;
}

// Comparison
bool Vector3::operator==(const Vector3& other) const
{
    for (int i = 0; i < 3; ++i)
        if (f[i] != other.f[i])
            return false;

    return true;
}

bool Vector3::operator<(const Vector3& other) const
{
    for (int i = 0; i < 3; ++i)
        if (f[i] >= other.f[i])
            return false;

    return true;
}

bool Vector3::operator>(const Vector3& other) const
{
    for (int i = 0; i < 3; ++i)
        if (f[i] <= other.f[i])
            return false;

    return true;
}

bool Vector3::operator<=(const Vector3& other) const
{
    for (int i = 0; i < 3; ++i)
        if (f[i] > other.f[i])
            return false;

    return true;
}

bool Vector3::operator>=(const Vector3& other) const
{
    for (int i = 0; i < 3; ++i)
        if (f[i] < other.f[i])
            return false;

    return true;
}



// VECTOR 4 //

Vector4::Vector4()
    : Vector4(0.0f)
{
}

Vector4::Vector4(float a)
    : f{a, a, a, a}
{
}

Vector4::Vector4(float a, float b, float c, float d)
    : f{a, b, c, d}
{
}

Vector4::Vector4(const Vector3& v, float w)
    : f{v.f[0], v.f[1], v.f[2], w}
{
}

Vector4::~Vector4()
{
}

Vector4::Vector4(const Vector4& other)
{
    memcpy(f, other.f, sizeof(float) * 4);
}

Vector4::Vector4(Vector4&& other)
    : Vector4()
{
    std::swap(f, other.f);
}

float* Vector4::Data()
{
    return f;
}

float Vector4::Length() const
{
    Vector4 temp(*this);

    return sqrtf(temp.Dot(temp));
}

void Vector4::Normalize()
{
    float len = Length();

    if (len)
        *this = *this / len;
}

Vector4& Vector4::operator= (const Vector4& other)
{
    Vector4 tmp(other);
    *this = std::move(tmp);

    return *this;
}

Vector4& Vector4::operator= (Vector4&& other)
{
    std::swap(f, other.f);

    return *this;
}

// Access operator
float Vector4::operator[](int index) const
{
    return f[index];
}

// Addition
Vector4& Vector4::operator+=(const Vector4& other)
{
    for (int i = 0; i < 4; ++i)
        f[i] += other.f[i];

    return *this;
}

Vector4& Vector4::operator+=(float value)
{
    for (auto& i : f)
        i += value;

    return *this;
}

const Vector4 Vector4::operator+(const Vector4& other) const
{
    return Vector4(*this) += other;
}

const Vector4 Vector4::operator+(float value) const
{
    return Vector4(*this) += value;
}

// Subtraction
Vector4& Vector4::operator-=(const Vector4& other)
{
    for (int i = 0; i < 4; ++i)
        f[i] -= other.f[i];

    return *this;
}

Vector4& Vector4::operator-=(float value)
{
    for (auto& i : f)
        i -= value;

    return *this;
}

const Vector4 Vector4::operator-(const Vector4& other) const
{
    return Vector4(*this) -= other;
}

const Vector4 Vector4::operator-(float value) const
{
    return Vector4(*this) -= value;
}

// Multiplication
Vector4& Vector4::operator*=(const Vector4& other)
{
    for (int i = 0; i < 4; ++i)
        f[i] *= other.f[i];

    return *this;
}

Vector4& Vector4::operator*=(float value)
{
    for (auto& i : f)
        i *= value;

    return *this;
}

const Vector4 Vector4::operator*(const Vector4& other) const
{
    return Vector4(*this) *= other;
}

const Vector4 Vector4::operator*(float value) const
{
    return Vector4(*this) *= value;
}

// Division
Vector4& Vector4::operator/=(const Vector4& other)
{
    for (int i = 0; i < 4; ++i)
        f[i] /= other.f[i];

    return *this;
}

Vector4& Vector4::operator/=(float value)
{
    for (auto& i : f)
        i /= value;

    return *this;
}

const Vector4 Vector4::operator/(const Vector4& other) const
{
    return Vector4(*this) /= other;
}

const Vector4 Vector4::operator/(float value) const
{
    return Vector4(*this) /= value;
}

// Power
Vector4& Vector4::operator^(float value)
{
    for (auto& i : f)
        i = pow(i, value);

    return *this;
}

// Products
float Vector4::Dot(const Vector4& other)
{
    Vector4 tmp(*this);
    tmp = tmp * other;

    float result = 0;
    for (auto i : tmp.f)
        result += i;

    return result;
}

Vector4 Vector4::Cross(const Vector4& other) const
{
    Vector4 result(*this);

    // assume this is a 3D Vector4 and calculate cross according to xyzzy mnemonic
    result.f[0] = this->f[1] * other.f[2] - this->f[2] * other.f[1];
    result.f[1] = this->f[2] * other.f[0] - this->f[0] * other.f[2];
    result.f[2] = this->f[0] * other.f[1] - this->f[1] * other.f[0];
    // result.f[3] remains unchanged

    return result;
}

// Comparison
bool Vector4::operator==(const Vector4& other) const
{
    for (int i = 0; i < 4; ++i)
        if (f[i] != other.f[i])
            return false;

    return true;
}

bool Vector4::operator<(const Vector4& other) const
{
    for (int i = 0; i < 4; ++i)
        if (f[i] >= other.f[i])
            return false;

    return true;
}

bool Vector4::operator>(const Vector4& other) const
{
    for (int i = 0; i < 4; ++i)
        if (f[i] <= other.f[i])
            return false;

    return true;
}

bool Vector4::operator<=(const Vector4& other) const
{
    for (int i = 0; i < 4; ++i)
        if (f[i] > other.f[i])
            return false;

    return true;
}

bool Vector4::operator>=(const Vector4& other) const
{
    for (int i = 0; i < 4; ++i)
        if (f[i] < other.f[i])
            return false;

    return true;
}

// Friendships
std::ostream& operator<<(std::ostream& os, const Vector3& v)
{
    os << "[" << v.f[0] << ", " << v.f[1] << ", " << v.f[2] << "]";
    return os;
}

std::ostream& operator<<(std::ostream& os, const Vector4& v)
{
    os << "[" << v.f[0] << ", " << v.f[1] << ", " << v.f[2] << ", " << v.f[3] << "]";
    return os;
}

} // namespace Math
} // namespace ABench
