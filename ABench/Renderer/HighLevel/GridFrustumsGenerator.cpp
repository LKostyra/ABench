#include "PCH.hpp"
#include "GridFrustumsGenerator.hpp"

#include "Renderer/LowLevel/Extensions.hpp"

#include "Math/Matrix.hpp"
#include "Math/Plane.hpp"


namespace {

struct GridFrustumsInfoBuffer
{
    ABench::Math::Matrix proj;
    uint32_t viewportWidth;
    uint32_t viewportHeight;
    uint32_t threadLimitX;
    uint32_t threadLimitY;
    uint32_t pixelsPerGridFrustum;
};

} // namespace

namespace ABench {
namespace Renderer {

GridFrustumsGenerator::GridFrustumsGenerator()
    : mGridFrustumsDataSet(VK_NULL_HANDLE)
    , mPipeline()
    , mDispatchCommandBuffer()
{
}

bool GridFrustumsGenerator::Init(const DevicePtr& device)
{
    mDevice = device;

    std::vector<DescriptorSetLayoutDesc> layoutDesc;
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    layoutDesc.push_back({VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, VK_SHADER_STAGE_COMPUTE_BIT, VK_NULL_HANDLE});
    mGridFrustumsDataSetLayout = Tools::CreateDescriptorSetLayout(mDevice, layoutDesc);
    if (!mGridFrustumsDataSetLayout)
        return false;

    std::vector<VkDescriptorSetLayout> layouts;
    layouts.push_back(mGridFrustumsDataSetLayout);
    mPipelineLayout = Tools::CreatePipelineLayout(mDevice, layouts);
    if (!mPipelineLayout)
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
                                     mGridFrustumsInfo.GetBuffer(), mGridFrustumsInfo.GetSize());

    if (!mDispatchCommandBuffer.Init(mDevice, DeviceQueueType::COMPUTE))
        return false;

    return true;
}

bool GridFrustumsGenerator::Generate(const GridFrustumsGenerationDesc& desc)
{
    uint32_t frustumsPerWidth = desc.viewportWidth / desc.pixelsPerGridFrustum;
    uint32_t frustumsPerHeight = desc.viewportHeight / desc.pixelsPerGridFrustum;

    if (desc.viewportWidth % desc.pixelsPerGridFrustum > 0)
        frustumsPerWidth++;
    if (desc.viewportHeight % desc.pixelsPerGridFrustum > 0)
        frustumsPerHeight++;

    GridFrustumsInfoBuffer info;
    info.proj = desc.projMat; // TODO inverse matrix here instead of GLSL shader
    info.viewportWidth = desc.viewportWidth;
    info.viewportHeight = desc.viewportHeight;
    info.threadLimitX = frustumsPerWidth;
    info.threadLimitY = frustumsPerHeight;
    info.pixelsPerGridFrustum = desc.pixelsPerGridFrustum;
    if (!mGridFrustumsInfo.Write(&info, sizeof(GridFrustumsInfoBuffer)))
        return false;

    // allocate new buffer for output data
    // size includes 4 planes per each frustum in the grid
    mGridFrustumsData.Free();
    BufferDesc gridFrustumsDataDesc;
    gridFrustumsDataDesc.data = nullptr;
    gridFrustumsDataDesc.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
    gridFrustumsDataDesc.type = BufferType::Dynamic;
    gridFrustumsDataDesc.dataSize = sizeof(Math::Plane) * 4 * frustumsPerWidth * frustumsPerHeight;
    if (!mGridFrustumsData.Init(mDevice, gridFrustumsDataDesc))
        return false;

    Tools::UpdateBufferDescriptorSet(mDevice, mGridFrustumsDataSet, VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1,
                                     mGridFrustumsData.GetBuffer(), mGridFrustumsData.GetSize());

    uint32_t dispatchThreadsX = frustumsPerWidth / desc.pixelsPerGridFrustum;
    uint32_t dispatchThreadsY = frustumsPerHeight / desc.pixelsPerGridFrustum;

    if (frustumsPerWidth % desc.pixelsPerGridFrustum > 0)
        dispatchThreadsX++;
    if (frustumsPerHeight % desc.pixelsPerGridFrustum > 0)
        dispatchThreadsY++;

    {
        ShaderMacros emptyMacros;

        mDispatchCommandBuffer.Begin();
        // TODO Buffer::Transition might be a better choice here, like in Texture
        mDispatchCommandBuffer.BufferBarrier(&mGridFrustumsData, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT, VK_PIPELINE_STAGE_COMPUTE_SHADER_BIT,
                                             0, VK_ACCESS_SHADER_WRITE_BIT,
                                             mDevice->GetQueueIndex(DeviceQueueType::COMPUTE), mDevice->GetQueueIndex(DeviceQueueType::COMPUTE));
        mDispatchCommandBuffer.BindPipeline(mPipeline.GetComputePipeline(emptyMacros), VK_PIPELINE_BIND_POINT_COMPUTE);
        mDispatchCommandBuffer.BindDescriptorSet(mGridFrustumsDataSet, VK_PIPELINE_BIND_POINT_COMPUTE, 0, mPipelineLayout);
        mDispatchCommandBuffer.Dispatch(dispatchThreadsX, dispatchThreadsY, 1);
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
