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
#include "ParticleEngine.hpp"
#include "DepthPrePass.hpp"
#include "LightCuller.hpp"
#include "ForwardPass.hpp"
#include "ParticlePass.hpp"


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
    VkRAII<VkSemaphore> mParticleEngineSem;
    VkRAII<VkSemaphore> mDepthSem;
    VkRAII<VkSemaphore> mCullingSem;
    VkRAII<VkSemaphore> mRenderSem;
    VkRAII<VkSemaphore> mParticlePassSem;
    VkRAII<VkFence> mParticleEngineFence;
    VkRAII<VkFence> mFrameFence;

    Math::Matrix mProjection;
    Math::Frustum mViewFrustum;
    VkRAII<VkDescriptorSetLayout> mVertexShaderLayout;
    VkDescriptorSet mVertexShaderSet;
    Buffer mVertexShaderCBuffer;
    RingBuffer mRingBuffer;
    Buffer mLightContainer;

    GridFrustumsGenerator mGridFrustumsGenerator;
    ParticleEngine mParticleEngine;
    DepthPrePass mDepthPrePass;
    LightCuller mLightCuller;
    ForwardPass mForwardPass;
    ParticlePass mParticlePass;

public:
    Renderer();
    ~Renderer();

    bool Init(const RendererDesc& desc);
    void Draw(const Scene::Scene& scene, const Scene::Camera& camera, float deltaTime);

    // this function should be used only when application finishes
    void WaitForAll() const;
};

} // namespace Renderer
} // namespace ABench
