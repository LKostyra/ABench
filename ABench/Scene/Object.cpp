#include "../PCH.hpp"
#include "Object.hpp"

namespace ABench {
namespace Scene {

Object::Object()
{
}

Object::~Object()
{
}

void Object::SetPosition(float x, float y, float z)
{
    mTransform = Math::CreateTranslationMatrix(Math::Vector(x, y, z, 1.0f));
}

} // namespace Scene
} // namespace ABench
