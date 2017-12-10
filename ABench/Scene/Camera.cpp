#include "PCH.hpp"
#include "Camera.hpp"
#include "Renderer/HighLevel/GridFrustumsGenerator.hpp"


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
    float aspect = static_cast<float>(desc.windowWidth) / static_cast<float>(desc.windowHeight);
    mProjection = Math::CreateRHProjectionMatrix(desc.fov, aspect, desc.nearZ, desc.farZ);
    mView = Math::CreateRHLookAtMatrix(desc.view.pos, desc.view.at, desc.view.up);
    mPosition = desc.view.pos;
    mAtPosition = desc.view.at;
    mUpVector = desc.view.up;

    Renderer::GridFrustumsGenerationDesc genDesc;
    genDesc.projMat = mProjection;
    genDesc.viewportWidth = desc.windowWidth;
    genDesc.viewportHeight = desc.windowHeight;
    return Renderer::GridFrustumsGenerator::Instance().Generate(genDesc, mGridFrustums);
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
