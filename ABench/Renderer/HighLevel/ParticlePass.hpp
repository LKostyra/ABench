#pragma once

#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/Tools.hpp"

#include "Scene/Scene.hpp"


namespace ABench {
namespace Renderer {

struct ParticlePassDesc
{
    Texture* targetTexture;
    Texture* depthTexture;
    Buffer* vertexShaderBuffer;
    std::string particleTexturePath;
    VkFormat outputFormat;
};

struct ParticlePassDrawDesc
{
    Buffer* particleDataBuffer;
    Buffer* emitterDataBuffer;
    uint32_t emitterCount;
    Math::Vector4 cameraPos;
    VkFence simulationFinishedFence;
    std::vector<VkPipelineStageFlags> waitFlags;
    std::vector<VkSemaphore> waitSems;
    VkSemaphore signalSem;
    VkFence fence;
};

class ParticlePass final
{
    DevicePtr mDevice;

    Buffer mParticleVertexBuffer;
    Texture mParticleTexture;
    Framebuffer mFramebuffer;
    VertexLayout mVertexLayout;
    MultiPipeline mPipeline;
    CommandBuffer mCommandBuffer;

    VkDescriptorSet mVertexShaderSet;
    VkDescriptorSet mFragmentShaderSet;
    VkRAII<VkSampler> mSampler;
    VkRAII<VkDescriptorSetLayout> mVertexShaderSetLayout;
    VkRAII<VkDescriptorSetLayout> mFragmentShaderSetLayout;
    VkRAII<VkRenderPass> mRenderPass;
    VkRAII<VkPipelineLayout> mPipelineLayout;

public:
    ParticlePass();

    bool Init(const DevicePtr& device, const ParticlePassDesc& desc);
    void Draw(const ParticlePassDrawDesc& desc);
};

} // namespace Renderer
} // namespace ABench
