#pragma once

#include "Renderer/LowLevel/Texture.hpp"
#include "Renderer/LowLevel/Framebuffer.hpp"
#include "Renderer/LowLevel/Buffer.hpp"
#include "Renderer/LowLevel/Shader.hpp"
#include "Renderer/LowLevel/CommandBuffer.hpp"
#include "Renderer/LowLevel/VertexLayout.hpp"
#include "Renderer/LowLevel/RingBuffer.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/Tools.hpp"

#include "Scene/Camera.hpp"
#include "Scene/Scene.hpp"

namespace ABench {
namespace Renderer {

struct ForwardPassDesc
{
    uint32_t width;
    uint32_t height;
    VkFormat outputFormat;
};

class ForwardPass final
{
    DevicePtr mDevice;

    Texture mTargetTexture;
    Texture mDepthTexture;
    Framebuffer mFramebuffer;
    VertexLayout mVertexLayout;
    MultiPipeline mPipeline;
    CommandBuffer mCommandBuffer;
    RingBuffer mRingBuffer;

    VkRAII<VkRenderPass> mRenderPass;
    VkRAII<VkPipelineLayout> mPipelineLayout;

    VkDescriptorSet mVertexShaderSet;
    VkDescriptorSet mFragmentShaderSet;
    VkDescriptorSet mAllShaderSet;
    Buffer mVertexShaderCBuffer;
    Buffer mAllShaderLightCBuffer;

public:
    ForwardPass();

    bool Init(const DevicePtr& device, const ForwardPassDesc& desc);
    void Draw(const Scene::Scene& scene, const Scene::Camera& camera, VkSemaphore waitSem, VkSemaphore signalSem, VkFence fence);

    ABENCH_INLINE Texture& GetTargetTexture()
    {
        return mTargetTexture;
    }
};

} // namespace Renderer
} // namespace ABench
