#include "PCH.hpp"
#include "Matrix.hpp"

#include "Common.hpp"

#include <algorithm>
#include <cstring>
#include <cmath>


namespace ABench {
namespace Math {

Matrix::Matrix()
    : Matrix(0.0f)
{
}

Matrix::Matrix(float a)
{
    for (int i = 0; i < 16; ++i)
        f[i] = a;
}

Matrix::Matrix(const float* a)
{
    for (int i = 0; i < 16; ++i)
        f[i] = a[i];
}

Matrix::Matrix(float a1, float a2, float a3, float a4,
               float b1, float b2, float b3, float b4,
               float c1, float c2, float c3, float c4,
               float d1, float d2, float d3, float d4)
    : f{a1,a2,a3,a4, b1,b2,b3,b4, c1,c2,c3,c4, d1,d2,d3,d4}
{
}

Matrix::Matrix(Vector4& a, Vector4& b, Vector4& c, Vector4& d)
{
    for (int i = 0; i < 4; ++i)
    {
        f[i] = a[i];
        f[i + 4 * 1] = b[i];
        f[i + 4 * 2] = c[i];
        f[i + 4 * 3] = d[i];
    }
}

Matrix::~Matrix()
{
}

Matrix::Matrix(const Matrix& other)
{
    memcpy(f, other.f, sizeof(float) * 16);
}

Matrix::Matrix(Matrix&& other)
    : Matrix()
{
    std::swap(f, other.f);
}

float* Matrix::Data()
{
    return f;
}

Matrix& Matrix::operator= (const Matrix& other)
{
    Matrix tmp(other);
    *this = std::move(tmp);

    return *this;
}

Matrix& Matrix::operator= (Matrix&& other)
{
    std::swap(f, other.f);

    return *this;
}

float Matrix::operator[](int index) const
{
    return f[index];
}

// Addition
Matrix& Matrix::operator+=(const Matrix& other)
{
    for (int i = 0; i < 4 * 4; ++i)
        f[i] += other.f[i];

    return *this;
}

Matrix& Matrix::operator+=(float value)
{
    for (auto& i : f)
        i += value;

    return *this;
}

const Matrix Matrix::operator+(const Matrix& other) const
{
    return Matrix(*this) += other;
}

const Matrix Matrix::operator+(float value) const
{
    return Matrix(*this) += value;
}

// Subtraction
Matrix& Matrix::operator-=(const Matrix& other)
{
    for (int i = 0; i < 4 * 4; ++i)
        f[i] -= other.f[i];

    return *this;
}

Matrix& Matrix::operator-=(float value)
{
    for (auto& i : f)
        i -= value;

    return *this;
}

const Matrix Matrix::operator-(const Matrix& other) const
{
    return Matrix(*this) -= other;
}

const Matrix Matrix::operator-(float value) const
{
    return Matrix(*this) -= value;
}

// Multiplication
Matrix& Matrix::operator*=(const Matrix& other)
{
    Matrix temp;
    float result;

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
        {
            result = 0;
            for (int k = 0; k < 4; ++k)
                result += f[4 * i + k] * other.f[4 * k + j];
            temp.f[4 * i + j] = result;
        }

    std::swap(f, temp.f);

    return *this;
}

Matrix& Matrix::operator*=(float value)
{
    for (auto& i : f)
        i *= value;

    return *this;
}

const Matrix Matrix::operator*(const Matrix& other) const
{
    return Matrix(*this) *= other;
}

const Matrix Matrix::operator*(float value) const
{
    return Matrix(*this) *= value;
}

// Division
Matrix& Matrix::operator/=(float value)
{
    for (auto& i : f)
        i /= value;

    return *this;
}

const Matrix Matrix::operator/(float value) const
{
    return Matrix(*this) /= value;
}

// Transposition
Matrix& Matrix::operator~()
{
    float tmp[16];

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            tmp[i * 4 + j] = f[i + 4 * j];

    std::swap(f, tmp);

    return *this;
}

// Power
Matrix& Matrix::operator^(float value)
{
    for (auto& i : f)
        i = pow(i, value);

    return *this;
}

// Comparison
bool Matrix::operator==(const Matrix& other) const
{
    for (int i = 0; i < 16; ++i)
        if (f[i] != other.f[i])
            return false;

    return true;
}

bool Matrix::operator<(const Matrix& other) const
{
    for (int i = 0; i < 16; ++i)
        if (f[i] >= other.f[i])
            return false;

    return true;
}

bool Matrix::operator>(const Matrix& other) const
{
    for (int i = 0; i < 16; ++i)
        if (f[i] <= other.f[i])
            return false;

    return true;
}

bool Matrix::operator<=(const Matrix& other) const
{
    for (int i = 0; i < 16; ++i)
        if (f[i] > other.f[i])
            return false;

    return true;
}

bool Matrix::operator>=(const Matrix& other) const
{
    for (int i = 0; i < 16; ++i)
        if (f[i] < other.f[i])
            return false;

    return true;
}


// External operators (cross-type)
Vector4 operator*(const Matrix& a, const Vector4& b)
{
    float result[] = {0, 0, 0, 0};

    for (int i = 0; i < 4; ++i)
        for (int j = 0; j < 4; ++j)
            result[i] += a[i * 4 + j] * b[j];

    return Vector4(result[0], result[1], result[2], result[3]);
}


Matrix CreateRHLookAtMatrix(const Vector4& pos, const Vector4& dir, const Vector4& up)
{
    Vector4 zAxis = pos - dir;
    zAxis.Normalize();

    Vector4 xAxis = up.Cross(zAxis);
    xAxis.Normalize();

    Vector4 yAxis = zAxis.Cross(xAxis);
    // No normalization needed here, since we cross two already normalized Vector4s.

    return Matrix(         xAxis[0],          yAxis[0],          zAxis[0], 0.0f,
                           xAxis[1],          yAxis[1],          zAxis[1], 0.0f,
                           xAxis[2],          yAxis[2],          zAxis[2], 0.0f,
                  -(xAxis.Dot(pos)), -(yAxis.Dot(pos)), -(zAxis.Dot(pos)), 1.0f);
}

Matrix CreateRHProjectionMatrix(const float fov, const float aspectRatio,
                                const float nearDist, const float farDist)
{
    float halfFovRad = (MATH_PIF/180.0f) * fov * 0.5f;
    float yScale = cosf(halfFovRad) / sinf(halfFovRad); // aka. ctg(halfFov)
    float xScale = yScale / aspectRatio;

    float distDiff = nearDist - farDist;
    float zVal1 = farDist / distDiff;
    float zVal2 = (nearDist * farDist) / distDiff;

    return Matrix(xScale,   0.0f,  0.0f, 0.0f,
                    0.0f, yScale,  0.0f, 0.0f,
                    0.0f,   0.0f, zVal1,-1.0f,
                    0.0f,   0.0f, zVal2, 0.0f);
}

Matrix CreateRotationMatrixX(const float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    return Matrix(1.0f, 0.0f, 0.0f, 0.0f,
                  0.0f,    c,   -s, 0.0f,
                  0.0f,    s,    c, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix CreateRotationMatrixY(const float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    return Matrix(   c, 0.0f,    s, 0.0f,
                  0.0f, 1.0f, 0.0f, 0.0f,
                    -s, 0.0f,    c, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix CreateRotationMatrixZ(const float angle)
{
    float c = cosf(angle);
    float s = sinf(angle);

    return Matrix(   c,   -s, 0.0f, 0.0f,
                     s,    c, 0.0f, 0.0f,
                  0.0f, 0.0f, 1.0f, 0.0f,
                  0.0f, 0.0f, 0.0f, 1.0f);
}

Matrix CreateTranslationMatrix(const Vector4& translation)
{
    Matrix result(MATRIX_IDENTITY);

    result.f[12] = translation[0];
    result.f[13] = translation[1];
    result.f[14] = translation[2];

    return result;
}

Matrix CreateScaleMatrix(const float scale)
{
    Matrix result(MATRIX_IDENTITY);

    result.f[0] = scale;
    result.f[5] = scale;
    result.f[10] = scale;

    return result;
}

Matrix CreateScaleMatrix(const float scaleX, const float scaleY, const float scaleZ)
{
    Matrix result(MATRIX_IDENTITY);

    result.f[0] = scaleX;
    result.f[5] = scaleY;
    result.f[10] = scaleZ;

    return result;
}

Matrix CreateScaleMatrix(const Vector4& scale)
{
    Matrix result(MATRIX_IDENTITY);

    result.f[0] = scale[0];
    result.f[5] = scale[1];
    result.f[10] = scale[2];

    return result;
}


} // namespace Math
} // namespace ABench
