#pragma once

#include "Vector.hpp"

namespace ABench {
namespace Math {

class Matrix
{
public:
    const float* Data() const;

    Matrix();
    Matrix(float a);
    Matrix(const float* a);
    Matrix(float a1, float a2, float a3, float a4,
           float b1, float b2, float b3, float b4,
           float c1, float c2, float c3, float c4,
           float d1, float d2, float d3, float d4);
    Matrix(Vector& a, Vector& b, Vector& c, Vector& d);
    Matrix(Vector4* a);
    ~Matrix();

    Matrix(const Matrix& other);
    Matrix(Matrix&& other);
    Matrix& operator= (const Matrix& other);
    Matrix& operator= (Matrix&& other);

    // Access operators
    float operator[](int index) const;

    // Addition
    Matrix& operator+=(const Matrix& other);
    Matrix& operator+=(float value);
    const Matrix operator+(const Matrix& other) const;
    const Matrix operator+(float value) const;

    // Subtraction
    Matrix& operator-=(const Matrix& other);
    Matrix& operator-=(float value);
    const Matrix operator-(const Matrix& other) const;
    const Matrix operator-(float value) const;

    // Multiplication
    Matrix& operator*=(const Matrix& other);
    Matrix& operator*=(float value);
    const Matrix operator*(const Matrix& other) const;
    const Matrix operator*(float value) const;

    // Division
    Matrix& operator/=(float value);
    const Matrix operator/(float value) const;

    // Transposition
    Matrix& operator~();

    // Power
    Matrix& operator^(float value);

    // Comparison
    bool operator==(const Matrix& other) const;
    bool operator<(const Matrix& other) const;
    bool operator>(const Matrix& other) const;
    bool operator<=(const Matrix& other) const;
    bool operator>=(const Matrix& other) const;

    // Friendship
    friend Vector operator*(const Matrix& a, const Vector& b);
    friend Matrix CreateTranslationMatrix(const Vector& translation);
    friend Matrix CreateScaleMatrix(const float scale);
    friend Matrix CreateScaleMatrix(const float scaleX, const float scaleY, const float scaleZ);

private:
    float f[4 * 4];
};


// External operators (cross-type)
Vector operator*(const Matrix& a, const Vector& b);

// Matrix creators
// TODO create LH versions
Matrix CreateRHLookAtMatrix(const Vector& pos, const Vector& dir, const Vector& up);
Matrix CreateRHProjectionMatrix(const float fov, const float aspectRatio,
                                const float nearDist, const float farDist);
Matrix CreateRotationMatrixX(const float angle);
Matrix CreateRotationMatrixY(const float angle);
Matrix CreateRotationMatrixZ(const float angle);
Matrix CreateTranslationMatrix(const Vector& translation);
Matrix CreateScaleMatrix(const float scale);
Matrix CreateScaleMatrix(const float scaleX, const float scaleY, const float scaleZ);

// Matrix constants
const Matrix MATRIX_IDENTITY(1.0f, 0.0f, 0.0f, 0.0f,
                             0.0f, 1.0f, 0.0f, 0.0f,
                             0.0f, 0.0f, 1.0f, 0.0f,
                             0.0f, 0.0f, 0.0f, 1.0f); ///< Identity matrix constant.

} // namespace Math
} // namespace ABench
