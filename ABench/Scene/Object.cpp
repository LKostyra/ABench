#include "PCH.hpp"
#include "Object.hpp"

namespace ABench {
namespace Scene {

Object::Object()
    : mTransform(ABench::Math::MATRIX_IDENTITY)
    , mComponent(nullptr)
{
}

Object::~Object()
{
}

} // namespace Scene
} // namespace ABench
