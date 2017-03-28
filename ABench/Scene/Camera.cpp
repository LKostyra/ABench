#include "../PCH.hpp"
#include "Camera.hpp"


namespace ABench {
namespace Scene {

Camera::Camera()
{
}

Camera::~Camera()
{
}

bool Camera::Init(const CameraDesc& desc)
{
    mProjection = Math::CreateRHProjectionMatrix(desc.fov, desc.aspect, desc.nearZ, desc.farZ);
    mView = Math::CreateRHLookAtMatrix(desc.view.pos, desc.view.at, desc.view.up);
    mPosition = desc.view.pos;
    mAtPosition = desc.view.at;
    mUpVector = desc.view.up;
    return true;
}

void Camera::Update(const CameraUpdateDesc& desc)
{
    mView = Math::CreateRHLookAtMatrix(desc.pos, desc.at, desc.up);
    mPosition = desc.pos;
    mAtPosition = desc.at;
    mUpVector = desc.up;
}

} // namespace Scene
} // namespace ABench
