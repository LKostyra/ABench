#include "PCH.hpp"
#include "Math/Vector.hpp"

using namespace ABench::Math;

namespace {

const Vector3 resultZero(0.0f, 0.0f, 0.0f);
const Vector3 resultOne(1.0f, 1.0f, 1.0f);
const Vector3 resultAdd(3.0f, 5.0f, 7.0f);
const Vector3 resultAdd2(3.0f, 4.0f, 5.0f);
const Vector3 resultSub(-1.0f, -1.0f, -1.0f);
const Vector3 resultSub2(-1.0f, 0.0f, 1.0f);
const Vector3 resultMul(2.0f, 6.0f, 12.0f);
const Vector3 resultMul2(2.0f, 4.0f, 6.0f);
const Vector3 resultDiv(0.5f, 0.5f, 0.5f);
const Vector3 resultDiv2(0.5f, 1.0f, 1.5f);
const Vector3 resultPow(8.0f);
const Vector3 resultCross(0.0f, 0.0f, 1.0f);
const Vector3 resultNorm(1.0f, 0.0f, 0.0f);

} // namespace

TEST(Vector3, Constructor)
{
    Vector3 a;
    EXPECT_TRUE(resultZero == a);
}

TEST(Vector3, ConstructorSingleFloat)
{
    Vector3 a(1.0f);

    EXPECT_TRUE(resultOne == a);
}

TEST(Vector3, AddVector3)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultAdd == (a + b));
}

TEST(Vector3, AddFloat)
{
    Vector3 a(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(resultAdd2 == (a + 2.0f));
}

TEST(Vector3, SubtractVector3)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultSub == (a - b));
}

TEST(Vector3, SubtractFloat)
{
    Vector3 a(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(resultSub2 == (a - 2.0f));
}

TEST(Vector3, MultiplyVector3)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(2.0f, 3.0f, 4.0f);

    EXPECT_TRUE(resultMul == (a * b));
}

TEST(Vector3, MultiplyFloat)
{
    Vector3 a(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(resultMul2 == (a * 2.0f));
}

TEST(Vector3, DivideVector3)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(2.0f, 4.0f, 6.0f);

    EXPECT_TRUE(resultDiv == (a / b));
}

TEST(Vector3, DivideFloat)
{
    Vector3 a(1.0f, 2.0f, 3.0f);

    EXPECT_TRUE(resultDiv2 == (a / 2.0f));
}

TEST(Vector3, Power)
{
    Vector3 a(2.0f);

    EXPECT_TRUE(resultPow == (a ^ 3.0f));
}

TEST(Vector3, Dot)
{
    Vector3 a(1.0f, 2.0f, 3.0f);
    Vector3 b(2.0f, 3.0f, 4.0f);

    EXPECT_EQ(20.0f, a.Dot(b));
}

TEST(Vector3, Cross)
{
    Vector3 a(1.0f, 0.0f, 0.0f);
    Vector3 b(0.0f, 1.0f, 0.0f);

    EXPECT_TRUE(resultCross == a.Cross(b));
}

TEST(Vector3, Length)
{
    Vector3 a(2.0f, 0.0f, 0.0f);

    EXPECT_EQ(2.0f, a.Length());
}

TEST(Vector3, Normalize)
{
    Vector3 a(2.0f, 0.0f, 0.0f);
    a.Normalize();

    EXPECT_TRUE(resultNorm == a);
}

TEST(Vector3, Equal)
{
    Vector3 a(1.0f);

    EXPECT_TRUE(a == a);
}

TEST(Vector3, Greater)
{
    Vector3 a(1.0f);
    Vector3 b(2.0f);

    ASSERT_GT(b, a);
}

TEST(Vector3, GreaterEqual)
{
    Vector3 a(1.0f);
    Vector3 b(2.0f);
    Vector3 c(1.0f);

    ASSERT_GE(b, a);
    ASSERT_GE(c, a);
}

TEST(Vector3, Less)
{
    Vector3 a(1.0f);
    Vector3 b(2.0f);

    ASSERT_LT(a, b);
}

TEST(Vector3, LessEqual)
{
    Vector3 a(1.0f);
    Vector3 b(2.0f);
    Vector3 c(1.0f);

    ASSERT_LE(a, b);
    ASSERT_LE(a, c);
}
