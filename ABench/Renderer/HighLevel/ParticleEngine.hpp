#pragma once

#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/CommandBuffer.hpp"
#include "Renderer/LowLevel/Tools.hpp"
#include "Renderer/LowLevel/Shader.hpp"
#include "Renderer/LowLevel/RingBuffer.hpp"

#include "Scene/Scene.hpp"


namespace ABench {
namespace Renderer {

struct ParticleEngineDispatchDesc
{
    Math::Vector4 cameraPos;
    float deltaTime;
    VkSemaphore signalSem;
    VkFence simulationFence;
};

class ParticleEngine final
{
    DevicePtr mDevice;

    Buffer mEngineParams;
    Buffer mEmitterData;
    Buffer mParticleData;
    Scene::EmitterCollection mEmitters;
    VkDescriptorSet mParticleEngineSet;
    VkRAII<VkDescriptorSetLayout> mParticleEngineSetLayout;
    VkRAII<VkPipelineLayout> mParticlePipelineLayout;
    Shader mParticleShader;
    Pipeline mParticlePipeline;
    CommandBuffer mParticleCommandBuffer;

    VkRAII<VkSemaphore> mParticleFinishedSem;
    RingBuffer mSortParams;
    VkDescriptorSet mSortSet;
    VkRAII<VkDescriptorSetLayout> mSortSetLayout;
    VkRAII<VkPipelineLayout> mSortPipelineLayout;
    Shader mSortShader;
    Pipeline mSortPipeline;
    CommandBuffer mSortCommandBuffer;
    std::random_device mRandomDevice;
    std::mt19937 mRandomGenerator;

public:
    ParticleEngine();

    bool Init(const DevicePtr& device);
    bool UpdateEmitters(const Scene::Scene& scene);
    void Dispatch(const ParticleEngineDispatchDesc& desc);

    ABENCH_INLINE Buffer* GetParticleDataBuffer()
    {
        return &mParticleData;
    }

    ABENCH_INLINE Buffer* GetEmitterDataBuffer()
    {
        return &mEmitterData;
    }
};

} // namespace Renderer
} // namespace ABench
