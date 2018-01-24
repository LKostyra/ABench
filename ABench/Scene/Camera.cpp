#include "PCH.hpp"
#include "Camera.hpp"


namespace ABench {
namespace Scene {

void Camera::Update(const CameraDesc& desc)
{
    mView = Math::CreateRHLookAtMatrix(desc.pos, desc.at, desc.up);
    mPosition = desc.pos;
    mAtPosition = desc.at;
    mUpVector = desc.up;
}

} // namespace Scene
} // namespace ABench
