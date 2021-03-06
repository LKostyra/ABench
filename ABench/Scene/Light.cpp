#include "PCH.hpp"
#include "Light.hpp"

namespace ABench {
namespace Scene {

Light::Light(const std::string& name)
    : Component(name)
{
}

Light::~Light()
{
}

} // namespace Scene
} // namespace ABench
