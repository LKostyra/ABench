#pragma once

#include "Buffer.hpp"
#include "MultiPipeline.hpp"
#include "DescriptorAllocator.hpp"
#include "CommandBuffer.hpp"


namespace ABench {
namespace Renderer {

class GridFrustumsGenerator
{
    uint32_t mFrustumsPerWidth;
    uint32_t mFrustumsPerHeight;
    BufferDesc mGridFrustumsDataDesc;
    Buffer mGridFrustumsData;
    Buffer mGridFrustumsInfo;
    VkDescriptorSet mGridFrustumsDataSet;
    VkDescriptorSetLayout mGridFrustumsDataSetLayout;
    VkPipelineLayout mPipelineLayout;
    MultiPipeline mPipeline;
    DescriptorAllocator mDescriptorAllocator;
    CommandBuffer mDispatchCommandBuffer;

public:
    GridFrustumsGenerator();
    ~GridFrustumsGenerator();

    bool Init();
    bool Generate(uint32_t viewportWidth, uint32_t viewportHeight);
};

} // namespace Renderer
} // namespace ABench
