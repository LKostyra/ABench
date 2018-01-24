#pragma once

#include "Renderer/LowLevel/Instance.hpp"
#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/Backbuffer.hpp"
#include "Renderer/LowLevel/Tools.hpp"

#include "Common/Window.hpp"

#include "Math/Frustum.hpp"

#include "Scene/Camera.hpp"
#include "Scene/Mesh.hpp"
#include "Scene/Scene.hpp"

#include "GridFrustumsGenerator.hpp"
#include "DepthPrePass.hpp"
#include "ForwardPass.hpp"


namespace ABench {
namespace Renderer {

struct RendererDesc
{
    bool debugEnable;
    bool debugVerbose;
    float fov;
    float nearZ;
    float farZ;
    Common::Window* window;
};

class Renderer final
{
    InstancePtr mInstance;
    DevicePtr mDevice;

    Backbuffer mBackbuffer;
    VkRAII<VkSemaphore> mImageAcquiredSem;
    VkRAII<VkSemaphore> mDepthFinishedSem;
    VkRAII<VkSemaphore> mRenderFinishedSem;
    VkRAII<VkFence> mDepthFence;
    VkRAII<VkFence> mFrameFence;

    Math::Matrix mProjection;
    Math::Frustum mViewFrustum;
    VkDescriptorSet mVertexShaderSet;
    Buffer mVertexShaderCBuffer;
    RingBuffer mRingBuffer;

    GridFrustumsGenerator mGridFrustumsGenerator;
    DepthPrePass mDepthPrePass;
    ForwardPass mForwardPass;

public:
    Renderer();
    ~Renderer();

    bool Init(const RendererDesc& desc);
    void Draw(const Scene::Scene& scene, const Scene::Camera& camera);

    // this function should be used only when application finishes
    void WaitForAll() const;
};

} // namespace Renderer
} // namespace ABench
