#include "PCH.hpp"
#include "GridFrustumsGenerator.hpp"

#include "Renderer/LowLevel/Tools.hpp"
#include "Renderer/LowLevel/Device.hpp"
#include "Renderer/LowLevel/Extensions.hpp"

#include "Math/Plane.hpp"


namespace {

const uint32_t PIXELS_PER_GRID_FRUSTUM = 16;

struct GridFrustumsInfoBuffer
{
    uint32_t viewportWidth;
    uint32_t viewportHeight;
};

} // namespace

namespace ABench {
namespace Renderer {

GridFrustumsGenerator::GridFrustumsGenerator()
    : mFrustumsPerWidth(0)
    , mFrustumsPerHeight(0)
    , mGridFrustumsDataDesc()
    , mGridFrustumsData()
    , mGridFrustumsDataSet(VK_NULL_HANDLE)
    , mGridFrustumsDataSetLayout(VK_NULL_HANDLE)
    , mPipelineLayout(VK_NULL_HANDLE)
    , mPipeline()
    , mDispatchCommandBuffer()
{
}

GridFrustumsGenerator::~GridFrustumsGenerator()
{
    if (mGridFrustumsDataSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(mDevice->GetDevice(), mGridFrustumsDataSetLayout, nullptr);
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(mDevice->GetDevice(), mPipelineLayout, nullptr);
}

bool GridFrustumsGenerator::Init(const DevicePtr& device)
{
    mDevice = device;

    mGridFrustumsDataDesc.data = nullptr;
    mGridFrustumsDataDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    mGridFrustumsDataDesc.type = BufferType::Dynamic;

    std::vector<DescriptorSetLayoutDesc> layoutDesc;
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    mGridFrustumsDataSetLayout = Tools::CreateDescriptorSetLayout(mDevice, layoutDesc);
    if (mGridFrustumsDataSetLayout == VK_NULL_HANDLE)
        return false;

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(mGridFrustumsDataSetLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    mGridFrustumsDataSet = DescriptorAllocator::Instance().AllocateDescriptorSet(mGridFrustumsDataSetLayout);
    if (mGridFrustumsDataSet == VK_NULL_HANDLE)
        return false;

    MultiComputePipelineDesc cpDesc;
    cpDesc.computeShader.path = "GridFrustumsGenerator.comp";
    cpDesc.pipelineDesc.pipelineLayout = mPipelineLayout;
    if (!mPipeline.Init(mDevice, cpDesc))
        return false;

    BufferDesc infoDesc;
    infoDesc.data = nullptr;
    infoDesc.dataSize = sizeof(GridFrustumsInfoBuffer);
    infoDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    infoDesc.type = BufferType::Dynamic;
    if (!mGridFrustumsInfo.Init(mDevice, infoDesc))
        return false;

    Tools::UpdateBufferDescriptorSet(mDevice, mGridFrustumsDataSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mGridFrustumsInfo.GetVkBuffer(), mGridFrustumsInfo.GetSize());

    if (!mDispatchCommandBuffer.Init(mDevice, DeviceQueueType::COMPUTE))
        return false;

    return true;
}

bool GridFrustumsGenerator::Generate(uint32_t viewportWidth, uint32_t viewportHeight)
{
    GridFrustumsInfoBuffer info;
    info.viewportWidth = viewportWidth;
    info.viewportHeight = viewportHeight;
    if (!mGridFrustumsInfo.Write(&info, sizeof(GridFrustumsInfoBuffer)))
        return false;

    mFrustumsPerWidth = viewportWidth / PIXELS_PER_GRID_FRUSTUM;
    mFrustumsPerHeight = viewportHeight / PIXELS_PER_GRID_FRUSTUM;

    if (viewportWidth % PIXELS_PER_GRID_FRUSTUM > 0)
        mFrustumsPerWidth++;
    if (viewportHeight % PIXELS_PER_GRID_FRUSTUM > 0)
        mFrustumsPerHeight++;

    mGridFrustumsDataDesc.dataSize = sizeof(Math::Plane) * mFrustumsPerWidth * mFrustumsPerHeight;
    mGridFrustumsData.Free();
    if (!mGridFrustumsData.Init(mDevice, mGridFrustumsDataDesc))
        return false;

    Tools::UpdateBufferDescriptorSet(mDevice, mGridFrustumsDataSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                     mGridFrustumsData.GetVkBuffer(), mGridFrustumsData.GetSize());

    {
        ShaderMacros emptyMacros;

        mDispatchCommandBuffer.Begin();
        mDispatchCommandBuffer.BindPipeline(mPipeline.GetComputePipeline(emptyMacros), VK_PIPELINE_BIND_POINT_COMPUTE);
        mDispatchCommandBuffer.BindDescriptorSet(mGridFrustumsDataSet, VK_PIPELINE_BIND_POINT_COMPUTE, 0, mPipelineLayout);
        mDispatchCommandBuffer.Dispatch(mFrustumsPerWidth, mFrustumsPerHeight, 1);
        mDispatchCommandBuffer.BufferBarrier(&mGridFrustumsData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             VK_ACCESS_SHADER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                                             mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));
        if (!mDispatchCommandBuffer.End())
            return false;

        mDevice->Execute(DeviceQueueType::COMPUTE, &mDispatchCommandBuffer);
        mDevice->Wait(DeviceQueueType::COMPUTE);
    }

    return true;
}

} // namespace Renderer
} // namespace ABench
