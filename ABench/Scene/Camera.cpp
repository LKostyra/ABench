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
    mGridFrustums = Renderer::GridFrustumsGenerator::Instance().Generate(genDesc);
    if (!mGridFrustums)
        return false;

    Math::FrustumDesc fdesc;
    fdesc.pos = desc.view.pos;
    fdesc.at = desc.view.at;
    fdesc.up = desc.view.up;
    fdesc.fov = desc.fov;
    fdesc.ratio = aspect;
    fdesc.nearZ = desc.nearZ;
    fdesc.farZ = desc.farZ;
    mFrustum.Init(fdesc);

    return true;
}

void Camera::Update(const CameraUpdateDesc& desc)
{
    mView = Math::CreateRHLookAtMatrix(desc.pos, desc.at, desc.up);
    mPosition = desc.pos;
    mAtPosition = desc.at;
    mUpVector = desc.up;
    mFrustum.Refresh(desc.pos, desc.at, desc.up);
}

} // namespace Scene
} // namespace ABench
