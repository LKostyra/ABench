#include "PCH.hpp"
#include "LightCuller.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"


namespace {

ABENCH_ALIGN(16)
struct GridLightData
{
    uint32_t offset;
    uint32_t count;
    uint32_t padding[2];
};

} // namespace


namespace ABench {
namespace Renderer {

LightCuller::LightCuller()
    : mDevice()
    , mCullingParams()
    , mCulledLights()
    , mGridLightData()
    , mLightCullerSet(VK_NULL_HANDLE)
    , mSampler()
    , mDescriptorSetLayout()
    , mPipelineLayout()
    , mShader()
    , mPipeline()
    , mCommandBuffer()
{
}

bool LightCuller::Init(const DevicePtr& device, const LightCullerDesc& desc)
{
    mDevice = device;

    mPixelsPerGridFrustum = desc.pixelsPerGridFrustum;
    mCullingParamsData.viewportWidth = desc.viewportWidth;
    mCullingParamsData.viewportHeight = desc.viewportHeight;
    mFrustumsPerWidth = desc.viewportWidth / mPixelsPerGridFrustum;
    mFrustumsPerHeight = desc.viewportHeight / mPixelsPerGridFrustum;

    if (desc.viewportWidth % mPixelsPerGridFrustum > 0)
        mFrustumsPerWidth++;
    if (desc.viewportHeight % mPixelsPerGridFrustum > 0)
        mFrustumsPerHeight++;

    BufferDesc bufDesc;
    bufDesc.dataSize = 1024 * mFrustumsPerWidth * mFrustumsPerHeight * 4;
    bufDesc.type = BufferType::Dynamic;
    bufDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (!mCulledLights.Init(mDevice, bufDesc))
        return false;

    bufDesc.dataSize = mFrustumsPerWidth * mFrustumsPerHeight * sizeof(GridLightData);
    if (!mGridLightData.Init(mDevice, bufDesc))
        return false;

    bufDesc.dataSize = sizeof(uint32_t) * 4;
    if (!mGlobalLightCounter.Init(mDevice, bufDesc))
        return false;

    bufDesc.dataSize = sizeof(CullingParams);
    bufDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (!mCullingParams.Init(mDevice, bufDesc))
        return false;


    mSampler = Tools::CreateSampler(mDevice);
    if (!mSampler)
        return false;


    std::vector<DescriptorSetLayoutDesc> layoutDesc;
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, mSampler});
    mDescriptorSetLayout = Tools::CreateDescriptorSetLayout(mDevice, layoutDesc);
    if (!mDescriptorSetLayout)
        return false;

    std::vector<VkDescriptorSetLayout> pipeDesc;
    pipeDesc.push_back(mDescriptorSetLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, pipeDesc);
    if (!mPipelineLayout)
        return false;

    ShaderDesc sDesc;
    sDesc.filename = "LightCuller.comp";
    sDesc.type = ShaderType::COMPUTE;
    if (!mShader.Init(mDevice, sDesc))
        return false;

    ComputePipelineDesc pDesc;
    pDesc.computeShader = &mShader;
    pDesc.pipelineLayout = mPipelineLayout;
    if (!mPipeline.Init(mDevice, pDesc))
        return false;

    if (!mCommandBuffer.Init(mDevice, DeviceQueueType::COMPUTE))
        return false;

    mLightCullerSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mDescriptorSetLayout);
    if (mLightCullerSet == VK_NULL_HANDLE)
        return false;

    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mCullingParams.GetBuffer(), mCullingParams.GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                     mGlobalLightCounter.GetBuffer(), mGlobalLightCounter.GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2,
                                     desc.gridFrustums->GetBuffer(), desc.gridFrustums->GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3,
                                     desc.lightContainer->GetBuffer(), desc.lightContainer->GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4,
                                     mCulledLights.GetBuffer(), mCulledLights.GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 5,
                                     mGridLightData.GetBuffer(), mGridLightData.GetSize());
    Tools::UpdateTextureDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 6,
                                      desc.depthTexture->GetView());

    mDepthTexture = desc.depthTexture;

    return true;
}

void LightCuller::Dispatch(const LightCullerDispatchDesc& desc)
{
    mCullingParamsData.projMat = desc.projMat;
    mCullingParamsData.viewMat = desc.viewMat;
    mCullingParamsData.lightCount = desc.lightCount;
    if (!mCullingParams.Write(&mCullingParamsData, sizeof(CullingParams)))
        LOGW("Light culler failed to update Light data");

    uint32_t lightCounter[] = { 0, 0, 0, 0 };
    if (!mGlobalLightCounter.Write(&lightCounter, sizeof(lightCounter)))
        LOGW("Light culler failed to initialize global light counter.");

    {
        mCommandBuffer.Begin();

        mCommandBuffer.BindPipeline(mPipeline.GetPipeline(), VK_PIPELINE_BIND_POINT_COMPUTE);
        mCommandBuffer.BindDescriptorSet(mLightCullerSet, VK_PIPELINE_BIND_POINT_COMPUTE, 0, mPipelineLayout);

        mCommandBuffer.BufferBarrier(&mCulledLights, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     0, VK_ACCESS_SHADER_WRITE_BIT,
                                     mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS), mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));
        mCommandBuffer.BufferBarrier(&mGridLightData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     0, VK_ACCESS_SHADER_WRITE_BIT,
                                     mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS), mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));

        mCommandBuffer.Dispatch(mFrustumsPerWidth, mFrustumsPerHeight, 1);

        mCommandBuffer.BufferBarrier(&mCulledLights, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_ACCESS_SHADER_WRITE_BIT, 0,
                                     mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS));
        mCommandBuffer.BufferBarrier(&mGridLightData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                     VK_ACCESS_SHADER_WRITE_BIT, 0,
                                     mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), mDevice->GetQueueIndex(DeviceQueueType::GRAPHICS));

        if (!mCommandBuffer.End())
            LOGW("Light culler failed to record command buffer");
    }

    mDevice->Execute(DeviceQueueType::COMPUTE, &mCommandBuffer,
                     static_cast<uint32_t>(desc.waitFlags.size()),
                     desc.waitFlags.data(), desc.waitSems.data(), desc.signalSem, VK_NULL_HANDLE);
}

} // namespace Renderer
} // namespace ABench
