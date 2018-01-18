#include "PCH.hpp"
#include "LightCuller.hpp"

#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "DescriptorLayoutManager.hpp"


namespace {

struct GridLightData
{
    uint32_t offset;
    uint32_t count;
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
    , mDescriptorSetLayout()
    , mPipelineLayout()
    , mPipeline()
    , mCommandBuffer()
{
}

bool LightCuller::Init(const DevicePtr& device, const LightCullerDesc& desc)
{
    mDevice = device;

    // TODO ASSUMES 1MB LIGHT LIST BUFFER - recalculate how much is actually needed
    BufferDesc bufDesc;
    bufDesc.dataSize = 1024 * 1024;
    bufDesc.concurrent = true;
    bufDesc.type = BufferType::Dynamic;
    bufDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    if (!mCulledLights.Init(mDevice, bufDesc))
        return false;

    mPixelsPerGridFrustum = desc.pixelsPerGridFrustum;
    mCullingParamsData.viewportWidth = desc.viewportWidth;
    mCullingParamsData.viewportHeight = desc.viewportHeight;
    mCullingParamsData.frustumsPerWidth = desc.viewportWidth / mPixelsPerGridFrustum;
    mCullingParamsData.frustumsPerHeight = desc.viewportHeight / mPixelsPerGridFrustum;

    if (desc.viewportWidth % mPixelsPerGridFrustum > 0)
        mCullingParamsData.frustumsPerWidth++;
    if (desc.viewportHeight % mPixelsPerGridFrustum > 0)
        mCullingParamsData.frustumsPerHeight++;

    bufDesc.dataSize = mCullingParamsData.frustumsPerWidth * mCullingParamsData.frustumsPerHeight * sizeof(GridLightData);
    if (!mGridLightData.Init(mDevice, bufDesc))
        return false;

    bufDesc.dataSize = sizeof(CullingParams);
    bufDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    if (!mCullingParams.Init(mDevice, bufDesc))
        return false;


    std::vector<DescriptorSetLayoutDesc> layoutDesc;
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_COMPUTE_BIT, DescriptorLayoutManager::Instance().GetSampler()});
    mDescriptorSetLayout = Tools::CreateDescriptorSetLayout(mDevice, layoutDesc);
    if (!mDescriptorSetLayout)
        return false;

    std::vector<VkDescriptorSetLayout> pipeDesc;
    pipeDesc.push_back(mDescriptorSetLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, pipeDesc);
    if (!mPipelineLayout)
        return false;

    MultiComputePipelineDesc mpDesc;
    mpDesc.computeShader.path = "LightCuller.comp";
    mpDesc.pipelineDesc.pipelineLayout = mPipelineLayout;
    if (!mPipeline.Init(mDevice, mpDesc))
        return false;

    if (!mCommandBuffer.Init(mDevice, DeviceQueueType::COMPUTE))
        return false;

    mLightCullerSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mDescriptorSetLayout);
    if (mLightCullerSet == VK_NULL_HANDLE)
        return false;

    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mCullingParams.GetBuffer(), mCullingParams.GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                     desc.gridFrustums->GetBuffer(), desc.gridFrustums->GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 2,
                                     desc.lightContainer->GetBuffer(), desc.lightContainer->GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 3,
                                     mCulledLights.GetBuffer(), mCulledLights.GetSize());
    Tools::UpdateBufferDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 4,
                                     mGridLightData.GetBuffer(), mGridLightData.GetSize());
    Tools::UpdateTextureDescriptorSet(mDevice, mLightCullerSet, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 5,
                                      desc.depthTexture->GetView());

    return true;
}

void LightCuller::Dispatch(const LightCullerDispatchDesc& desc)
{
    // TODO we can probably record this stuff once
    mCullingParamsData.lightCount = desc.lightCount;
    if (!mCullingParams.Write(&mCullingParamsData, sizeof(CullingParams)))
        LOGW("Light culler failed to update Light data");

    uint32_t dispatchThreadsX = mCullingParamsData.frustumsPerWidth / mPixelsPerGridFrustum;
    uint32_t dispatchThreadsY = mCullingParamsData.frustumsPerHeight / mPixelsPerGridFrustum;

    if (mCullingParamsData.frustumsPerWidth % mPixelsPerGridFrustum > 0)
        dispatchThreadsX++;
    if (mCullingParamsData.frustumsPerHeight % mPixelsPerGridFrustum > 0)
        dispatchThreadsY++;

    {
        ShaderMacros emptyMacros;

        mCommandBuffer.Begin();
        mCommandBuffer.BindPipeline(mPipeline.GetComputePipeline(emptyMacros), VK_PIPELINE_BIND_POINT_COMPUTE);
        mCommandBuffer.BindDescriptorSet(mLightCullerSet, VK_PIPELINE_BIND_POINT_COMPUTE, 0, mPipelineLayout);
        mCommandBuffer.Dispatch(dispatchThreadsX, dispatchThreadsY, 1);

        if (!mCommandBuffer.End())
            LOGW("Light culler failed to record command buffer");
    }

    mDevice->Execute(DeviceQueueType::COMPUTE, &mCommandBuffer,
                     static_cast<uint32_t>(desc.waitFlags.size()),
                     desc.waitFlags.data(), desc.waitSems.data(), desc.signalSem, VK_NULL_HANDLE);
}

} // namespace Renderer
} // namespace ABench
