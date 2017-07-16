#include "PCH.hpp"
#include "Light.hpp"

namespace ABench {
namespace Scene {

Light::Light(const std::string& name)
    : mDiffuseIntensity(1.0f)
    , mName(name)
{
}

Light::~Light()
{
}

} // namespace Scene
} // namespace ABench
