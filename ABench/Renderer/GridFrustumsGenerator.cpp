#include "PCH.hpp"
#include "GridFrustumsGenerator.hpp"
#include "Tools.hpp"
#include "Device.hpp"
#include "Extensions.hpp"
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

extern Device* gDevice;

GridFrustumsGenerator::GridFrustumsGenerator()
    : mFrustumsPerWidth(0)
    , mFrustumsPerHeight(0)
    , mGridFrustumsDataDesc()
    , mGridFrustumsData()
    , mGridFrustumsDataSet(VK_NULL_HANDLE)
    , mGridFrustumsDataSetLayout(VK_NULL_HANDLE)
    , mPipelineLayout(VK_NULL_HANDLE)
    , mPipeline()
    , mDescriptorAllocator()
    , mDispatchCommandBuffer()
{
}

GridFrustumsGenerator::~GridFrustumsGenerator()
{
    if (mGridFrustumsDataSetLayout != VK_NULL_HANDLE)
        vkDestroyDescriptorSetLayout(gDevice->GetDevice(), mGridFrustumsDataSetLayout, nullptr);
    if (mPipelineLayout != VK_NULL_HANDLE)
        vkDestroyPipelineLayout(gDevice->GetDevice(), mPipelineLayout, nullptr);
}

bool GridFrustumsGenerator::Init()
{
    mGridFrustumsDataDesc.data = nullptr;
    mGridFrustumsDataDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    mGridFrustumsDataDesc.type = BufferType::Dynamic;

    std::vector<DescriptorSetLayoutDesc> layoutDesc;
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    mGridFrustumsDataSetLayout = Tools::CreateDescriptorSetLayout(layoutDesc);
    if (mGridFrustumsDataSetLayout == VK_NULL_HANDLE)
        return false;

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(mGridFrustumsDataSetLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(layouts);
    if (mPipelineLayout == VK_NULL_HANDLE)
        return false;

    DescriptorAllocatorDesc daDesc;
    daDesc.limits[VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER] = 1;
    daDesc.limits[VK_DESCRIPTOR_TYPE_STORAGE_BUFFER] = 1;
    if (!mDescriptorAllocator.Init(gDevice->GetDevice(), daDesc))
        return false;

    mGridFrustumsDataSet = mDescriptorAllocator.AllocateDescriptorSet(mGridFrustumsDataSetLayout);
    if (mGridFrustumsDataSet == VK_NULL_HANDLE)
        return false;

    MultiComputePipelineDesc cpDesc;
    cpDesc.computeShader.path = "GridFrustumsGenerator.comp";
    cpDesc.pipelineDesc.pipelineLayout = mPipelineLayout;
    if (!mPipeline.Init(cpDesc))
        return false;

    if (!mDispatchCommandBuffer.Init(gDevice->GetCommandPool(DeviceQueueType::COMPUTE)))
        return false;

    BufferDesc infoDesc;
    infoDesc.data = nullptr;
    infoDesc.dataSize = sizeof(GridFrustumsInfoBuffer);
    infoDesc.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    infoDesc.type = BufferType::Dynamic;
    if (!mGridFrustumsInfo.Init(infoDesc))
        return false;

    Tools::UpdateBufferDescriptorSet(mGridFrustumsDataSet, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 0,
                                     mGridFrustumsInfo.GetVkBuffer(), mGridFrustumsInfo.GetSize());

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
    if (!mGridFrustumsData.Init(mGridFrustumsDataDesc))
        return false;

    Tools::UpdateBufferDescriptorSet(mGridFrustumsDataSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                     mGridFrustumsData.GetVkBuffer(), mGridFrustumsData.GetSize());

    {
        ShaderMacros emptyMacros;

        mDispatchCommandBuffer.Begin();
        mDispatchCommandBuffer.BindPipeline(mPipeline.GetComputePipeline(emptyMacros), VK_PIPELINE_BIND_POINT_COMPUTE);
        mDispatchCommandBuffer.BindDescriptorSet(mGridFrustumsDataSet, 0, mPipelineLayout);
        mDispatchCommandBuffer.Dispatch(1, 1, 0);
        mDispatchCommandBuffer.End();

        gDevice->Execute(DeviceQueueType::COMPUTE, &mDispatchCommandBuffer);
        gDevice->Wait(DeviceQueueType::COMPUTE);
    }

    return true;
}

} // namespace Renderer
} // namespace ABench
