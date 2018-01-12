#include "PCH.hpp"
#include "Object.hpp"

namespace ABench {
namespace Scene {

Object::Object()
    : mTransform()
    , mPosition(0.0f)
    , mScale(1.0f)
    , mComponent(nullptr)
    , mToRender(false)
{
}

Object::~Object()
{
}

} // namespace Scene
} // namespace ABench
