#pragma once

#include "Renderer/LowLevel/Buffer.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/CommandBuffer.hpp"


namespace ABench {
namespace Renderer {

class GridFrustumsGenerator
{
    DevicePtr mDevice;

    uint32_t mFrustumsPerWidth;
    uint32_t mFrustumsPerHeight;
    BufferDesc mGridFrustumsDataDesc;
    Buffer mGridFrustumsData;
    Buffer mGridFrustumsInfo;
    VkDescriptorSet mGridFrustumsDataSet;
    VkDescriptorSetLayout mGridFrustumsDataSetLayout;
    VkPipelineLayout mPipelineLayout;
    MultiPipeline mPipeline;
    CommandBuffer mDispatchCommandBuffer;

public:
    GridFrustumsGenerator();
    ~GridFrustumsGenerator();

    bool Init(const DevicePtr& device);
    bool Generate(uint32_t viewportWidth, uint32_t viewportHeight);
};

} // namespace Renderer
} // namespace ABench
