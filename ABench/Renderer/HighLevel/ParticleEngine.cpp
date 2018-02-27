#include "PCH.hpp"
#include "ParticleEngine.hpp"

#include "Math/Vector.hpp"
#include "Renderer/LowLevel/DescriptorAllocator.hpp"


namespace {

ABENCH_ALIGN(16)
struct ParticleEngineParams
{
    ABench::Math::Vector4 cameraPos;
    float deltaTime;
    uint32_t emitterCount;
    float randomSeedA;
    float randomSeedB;
};

ABENCH_ALIGN(16)
struct ParticleData
{
    ABench::Math::Vector4 pos;
    ABench::Math::Vector3 color;
    float lifeTimer;
    ABench::Math::Vector4 newpos;
    ABench::Math::Vector3 vel;
    float cameraDistance;
};

ABENCH_ALIGN(16)
struct SortParams
{
    uint32_t sortSpace;
    uint32_t offset;
    uint32_t boxSize;
    uint32_t compareDist;
};

} // namespace


namespace ABench {
namespace Renderer {

ParticleEngine::ParticleEngine()
    : mDevice()
    , mParticleEngineSet(VK_NULL_HANDLE)
    , mSortSet(VK_NULL_HANDLE)
    , mRandomDevice()
    , mRandomGenerator(mRandomDevice())
{
}

bool ParticleEngine::Init(const DevicePtr& device)
{
    mDevice = device;

    // Particle-related parameters
    {
        std::vector<DescriptorSetLayoutDesc> setLayoutDescs;
        setLayoutDescs.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
        setLayoutDescs.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
        setLayoutDescs.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
        mParticleEngineSetLayout = Tools::CreateDescriptorSetLayout(mDevice, setLayoutDescs);
        if (!mParticleEngineSetLayout)
            return false;

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.push_back(mParticleEngineSetLayout);
        mParticlePipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
        if (!mParticlePipelineLayout)
            return false;

        mParticleEngineSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mParticleEngineSetLayout);
        if (mParticleEngineSet == VK_NULL_HANDLE)
            return false;

        ShaderDesc sDesc;
        sDesc.filename = "ParticleEngine.comp";
        sDesc.type = ShaderType::COMPUTE;
        if (!mParticleShader.Init(mDevice, sDesc))
            return false;

        ComputePipelineDesc cpDesc;
        cpDesc.computeShader = &mParticleShader;
        cpDesc.pipelineLayout = mParticlePipelineLayout;
        if (!mParticlePipeline.Init(mDevice, cpDesc))
            return false;

        BufferDesc bDesc;
        bDesc.data = nullptr;
        bDesc.dataSize = sizeof(ParticleEngineParams);
        bDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
        bDesc.type = BufferType::Dynamic;
        bDesc.concurrent = false;
        if (!mEngineParams.Init(mDevice, bDesc))
            return false;

        Tools::UpdateBufferDescriptorSet(mDevice, mParticleEngineSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                         0, mEngineParams.GetBuffer(), mEngineParams.GetSize());

        if (!mParticleCommandBuffer.Init(mDevice, DeviceQueueType::COMPUTE))
            return false;
    }


    // Sorting-related parameters
    {
        mParticleFinishedSem = Tools::CreateSem(mDevice);
        if (!mParticleFinishedSem)
            return false;

        std::vector<DescriptorSetLayoutDesc> setLayoutDescs;
        setLayoutDescs.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
        setLayoutDescs.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
        mSortSetLayout = Tools::CreateDescriptorSetLayout(mDevice, setLayoutDescs);
        if (!mSortSetLayout)
            return false;

        std::vector<VkDescriptorSetLayout> layouts;
        layouts.push_back(mSortSetLayout);
        mSortPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
        if (!mSortPipelineLayout)
            return false;

        mSortSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mSortSetLayout);
        if (mSortSet == VK_NULL_HANDLE)
            return false;

        ShaderDesc sDesc;
        sDesc.filename = "BitonicSorter.comp";
        sDesc.type = ShaderType::COMPUTE;
        if (!mSortShader.Init(mDevice, sDesc))
            return false;

        ComputePipelineDesc cpDesc;
        cpDesc.computeShader = &mSortShader;
        cpDesc.pipelineLayout = mSortPipelineLayout;
        if (!mSortPipeline.Init(mDevice, cpDesc))
            return false;

        if (!mSortParams.Init(mDevice, 1024 * 32))
            return false;

        Tools::UpdateBufferDescriptorSet(mDevice, mSortSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
                                         0, mSortParams.GetBuffer(), sizeof(SortParams));

        if (!mSortCommandBuffer.Init(mDevice, DeviceQueueType::COMPUTE))
            return false;
    }

    return true;
}

bool ParticleEngine::UpdateEmitters(const Scene::Scene& scene)
{
    bool bufferNeedsUpdate = false;

    scene.ForEachEmitter([&](const Scene::Emitter* e) -> bool {
        auto it = std::find_if(mEmitters.begin(), mEmitters.end(), [e](const Scene::Emitter* em) {
            return (e == em);
        });

        if (it == mEmitters.end())
        {
            mEmitters.emplace_back(e);
            bufferNeedsUpdate = true;
            return true;
        }

        return true;
    });

    if (bufferNeedsUpdate)
    {
        // collect how much space we need
        VkDeviceSize emitterBufSize = mEmitters.size() * sizeof(Scene::EmitterData);
        VkDeviceSize particleBufSize = 0;
        for (auto& e: mEmitters)
            particleBufSize += sizeof(ParticleData) * e->GetParticleLimit();

        LOGD("Updating Particle Engine Buffers to sizes " << emitterBufSize << " " << particleBufSize);

        mEmitterData.Free();
        BufferDesc desc;
        desc.data = nullptr;
        desc.dataSize = emitterBufSize;
        desc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
        desc.type = BufferType::Dynamic;
        if (!mEmitterData.Init(mDevice, desc))
        {
            LOGE("Particle Data Buffer reallocation failed!");
            return false;
        }

        mParticleData.Free();
        desc.dataSize = particleBufSize;
        desc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
        desc.concurrent = false;
        if (!mParticleData.Init(mDevice, desc))
        {
            LOGE("Particle Data Buffer reallocation failed!");
            return false;
        }

        uint32_t emitterDataOffset = 0;
        uint32_t particleDataOffset = 0;
        for (auto e: mEmitters)
        {
            // update Emitter Data buffer
            e->SetParticleDataOffset(particleDataOffset);
            const Scene::EmitterData& emData = e->GetData();
            mEmitterData.Write(&emData, sizeof(Scene::EmitterData), emitterDataOffset);

            // update offset in Buffer
            emitterDataOffset += sizeof(Scene::EmitterData);
            particleDataOffset += sizeof(ParticleData) * e->GetParticleLimit();
        }

        std::vector<char> emptyBuf(particleDataOffset);
        memset(emptyBuf.data(), 0x0, particleDataOffset);
        mParticleData.Write(emptyBuf.data(), particleDataOffset);

        Tools::UpdateBufferDescriptorSet(mDevice, mParticleEngineSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         1, mEmitterData.GetBuffer(), mEmitterData.GetSize());
        Tools::UpdateBufferDescriptorSet(mDevice, mParticleEngineSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         2, mParticleData.GetBuffer(), mParticleData.GetSize());

        Tools::UpdateBufferDescriptorSet(mDevice, mSortSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
                                         1, mParticleData.GetBuffer(), mParticleData.GetSize());
    }

    return true;
}

void ParticleEngine::Dispatch(const ParticleEngineDispatchDesc& desc)
{
    if (mEmitters.empty())
        return;

    ParticleEngineParams engineParams;
    engineParams.cameraPos = desc.cameraPos;
    engineParams.deltaTime = desc.deltaTime;
    engineParams.emitterCount = static_cast<uint32_t>(mEmitters.size());
    engineParams.randomSeedA = static_cast<float>(mRandomGenerator()) / static_cast<float>(mRandomGenerator.max());
    engineParams.randomSeedB = static_cast<float>(mRandomGenerator()) / static_cast<float>(mRandomGenerator.max());
    if (!mEngineParams.Write(&engineParams, sizeof(ParticleEngineParams)))
    {
        LOGW("Failed to update Particle Engine parameters for dispatch");
        return;
    }

    {
        mParticleCommandBuffer.Begin();

        mParticleCommandBuffer.BindPipeline(mParticlePipeline.GetPipeline(), VK_PIPELINE_BIND_POINT_COMPUTE);
        mParticleCommandBuffer.BindDescriptorSet(mParticleEngineSet, VK_PIPELINE_BIND_POINT_COMPUTE, 0, mParticlePipelineLayout);

        mParticleCommandBuffer.BufferBarrier(&mParticleData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             0,
                                             VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                             mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS), mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));
        mParticleCommandBuffer.BufferBarrier(&mEmitterData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             0,
                                             VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                             VK_QUEUE_FAMILY_IGNORED, mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));

        mParticleCommandBuffer.Dispatch(static_cast<uint32_t>(mEmitters.size()), 1, 1);
        mParticleCommandBuffer.BufferBarrier(&mEmitterData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                             0,
                                             mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), VK_QUEUE_FAMILY_IGNORED);

        if (!mParticleCommandBuffer.End())
            LOGW("Particle Engine failed to record command buffer for Particle update");
    }

    mDevice->Execute(DeviceQueueType::COMPUTE, &mParticleCommandBuffer,
                     0, nullptr, nullptr, mParticleFinishedSem, desc.simulationFence);

    {
        mSortCommandBuffer.Begin();
        mSortCommandBuffer.BindPipeline(mSortPipeline.GetPipeline(), VK_PIPELINE_BIND_POINT_COMPUTE);

        SortParams params;
        for (auto& e: mEmitters)
        {
            params.sortSpace = e->GetParticleLimit();
            params.offset = e->GetParticleDataOffset();

            for (uint32_t boxSize = 2; boxSize <= e->GetParticleLimit(); boxSize <<= 1)
            {
                params.boxSize = boxSize;
                for (uint32_t compareDist = (boxSize >> 1); compareDist > 0; compareDist >>= 1)
                {
                    params.compareDist = compareDist;
                    uint32_t offset = mSortParams.Write(&params, sizeof(SortParams));
                    mSortCommandBuffer.BindDescriptorSet(mSortSet, VK_PIPELINE_BIND_POINT_COMPUTE, 0, mSortPipelineLayout, offset);
                    mSortCommandBuffer.BufferBarrier(&mParticleData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                                     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                                     VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT,
                                                     VK_QUEUE_FAMILY_IGNORED, VK_QUEUE_FAMILY_IGNORED);
                    mSortCommandBuffer.Dispatch(1, 1, 1);
                }
            }
        }

        mSortCommandBuffer.BufferBarrier(&mParticleData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                         VK_ACCESS_SHADER_READ_BIT | VK_ACCESS_SHADER_WRITE_BIT, 0,
                                         mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS));

        if (!mSortCommandBuffer.End())
            LOGW("Particle Engine failed to record command buffer for Particle sorting");
    }

    VkPipelineStageFlags waitFlag = VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT;
    VkSemaphore waitSem = mParticleFinishedSem;
    mDevice->Execute(DeviceQueueType::COMPUTE, &mSortCommandBuffer,
                     1, &waitFlag, &waitSem, desc.signalSem, VK_NULL_HANDLE);

    mSortParams.MarkFinishedFrame();
}

} // namespace Renderer
} // namespace ABench
