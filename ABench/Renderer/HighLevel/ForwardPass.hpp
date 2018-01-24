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
    Texture* depthTexture;
    RingBuffer* ringBufferPtr;

    ForwardPassDesc()
        : width(0)
        , height(0)
        , outputFormat(VK_FORMAT_UNDEFINED)
        , depthTexture(nullptr)
        , ringBufferPtr(nullptr)
    {
    }
};

struct ForwardPassDrawDesc
{
    RingBuffer* ringBufferPtr;
    VkDescriptorSet vertexShaderSet;
    std::vector<VkPipelineStageFlags> waitFlags;
    std::vector<VkSemaphore> waitSems;
    VkSemaphore signalSem;
    VkFence fence;

    ForwardPassDrawDesc()
        : ringBufferPtr(nullptr)
        , vertexShaderSet(VK_NULL_HANDLE)
        , waitFlags()
        , waitSems()
        , signalSem(VK_NULL_HANDLE)
        , fence(VK_NULL_HANDLE)
    {
    }
};

class ForwardPass final
{
    DevicePtr mDevice;

    Texture mTargetTexture;
    Texture* mDepthTexture;
    Framebuffer mFramebuffer;
    VertexLayout mVertexLayout;
    MultiPipeline mPipeline;
    CommandBuffer mCommandBuffer;

    VkRAII<VkRenderPass> mRenderPass;
    VkRAII<VkPipelineLayout> mPipelineLayout;

    VkDescriptorSet mFragmentShaderSet;
    VkDescriptorSet mAllShaderSet;
    Buffer mAllShaderLightCBuffer;

public:
    ForwardPass();

    bool Init(const DevicePtr& device, const ForwardPassDesc& desc);
    void Draw(const Scene::Scene& scene, const Scene::Camera& camera, const ForwardPassDrawDesc& desc);

    ABENCH_INLINE Texture& GetTargetTexture()
    {
        return mTargetTexture;
    }
};

} // namespace Renderer
} // namespace ABench
