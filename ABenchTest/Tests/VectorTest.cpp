#include "PCH.hpp"
#include "Math/Vector.hpp"

using namespace ABench::Math;

namespace {

const Vector resultZero(0.0f, 0.0f, 0.0f, 0.0f);
const Vector resultOne(1.0f, 1.0f, 1.0f, 1.0f);
const Vector resultAdd(3.0f, 5.0f, 7.0f, 9.0f);
const Vector resultAdd2(3.0f, 4.0f, 5.0f, 6.0f);
const Vector resultSub(-1.0f, -1.0f, -1.0f, -1.0f);
const Vector resultSub2(-1.0f, 0.0f, 1.0f, 2.0f);
const Vector resultMul(2.0f, 6.0f, 12.0f, 20.0f);
const Vector resultMul2(2.0f, 4.0f, 6.0f, 8.0f);
const Vector resultDiv(0.5f, 0.5f, 0.5f, 0.5f);
const Vector resultDiv2(0.5f, 1.0f, 1.5f, 2.0f);
const Vector resultPow(8.0f);
const Vector resultCross(0.0f, 0.0f, 1.0f, 0.0f);
const Vector resultNorm(1.0f, 0.0f, 0.0f, 0.0f);

} // namespace

TEST(Vector, Constructor)
{
    Vector a;
    EXPECT_TRUE(resultZero == a);
}

TEST(Vector, ConstructorSingleFloat)
{
    Vector a(1.0f);

    EXPECT_TRUE(resultOne == a);
}

TEST(Vector, AddVector)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector b(2.0f, 3.0f, 4.0f, 5.0f);

    EXPECT_TRUE(resultAdd == (a + b));
}

TEST(Vector, AddFloat)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultAdd2 == (a + 2.0f));
}

TEST(Vector, SubtractVector)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector b(2.0f, 3.0f, 4.0f, 5.0f);

    EXPECT_TRUE(resultSub == (a - b));
}

TEST(Vector, SubtractFloat)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultSub2 == (a - 2.0f));
}

TEST(Vector, MultiplyVector)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector b(2.0f, 3.0f, 4.0f, 5.0f);

    EXPECT_TRUE(resultMul == (a * b));
}

TEST(Vector, MultiplyFloat)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultMul2 == (a * 2.0f));
}

TEST(Vector, DivideVector)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector b(2.0f, 4.0f, 6.0f, 8.0f);

    EXPECT_TRUE(resultDiv == (a / b));
}

TEST(Vector, DivideFloat)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultDiv2 == (a / 2.0f));
}

TEST(Vector, Power)
{
    Vector a(2.0f);

    EXPECT_TRUE(resultPow == (a ^ 3.0f));
}

TEST(Vector, Dot)
{
    Vector a(1.0f, 2.0f, 3.0f, 4.0f);
    Vector b(2.0f, 3.0f, 4.0f, 5.0f);

    EXPECT_EQ(40.0f, a.Dot(b));
}

TEST(Vector, Cross)
{
    Vector a(1.0f, 0.0f, 0.0f, 0.0f);
    Vector b(0.0f, 1.0f, 0.0f, 0.0f);

    EXPECT_TRUE(resultCross == a.Cross(b));
}

TEST(Vector, Length)
{
    Vector a(2.0f, 0.0f, 0.0f, 0.0f);

    EXPECT_EQ(2.0f, a.Length());
}

TEST(Vector, Normalize)
{
    Vector a(2.0f, 0.0f, 0.0f, 0.0f);
    a.Normalize();

    EXPECT_TRUE(resultNorm == a);
}

TEST(Vector, Equal)
{
    Vector a(1.0f);

    EXPECT_TRUE(a == a);
}

TEST(Vector, Greater)
{
    Vector a(1.0f);
    Vector b(2.0f);

    ASSERT_GT(b, a);
}

TEST(Vector, GreaterEqual)
{
    Vector a(1.0f);
    Vector b(2.0f);
    Vector c(1.0f);

    ASSERT_GE(b, a);
    ASSERT_GE(c, a);
}

TEST(Vector, Less)
{
    Vector a(1.0f);
    Vector b(2.0f);

    ASSERT_LT(a, b);
}

TEST(Vector, LessEqual)
{
    Vector a(1.0f);
    Vector b(2.0f);
    Vector c(1.0f);

    ASSERT_LE(a, b);
    ASSERT_LE(a, c);
}
