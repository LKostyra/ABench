#pragma once

#include "Renderer/LowLevel/Buffer.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/CommandBuffer.hpp"
#include "Math/Matrix.hpp"

namespace ABench {
namespace Renderer {

struct GridFrustumsGenerationDesc
{
    ABench::Math::Matrix projMat;
    uint32_t viewportWidth;
    uint32_t viewportHeight;
};

class GridFrustumsGenerator
{
    DevicePtr mDevice;

    Buffer mGridFrustumsInfo;
    VkDescriptorSet mGridFrustumsDataSet;
    VkDescriptorSetLayout mGridFrustumsDataSetLayout;
    VkPipelineLayout mPipelineLayout;
    MultiPipeline mPipeline;
    CommandBuffer mDispatchCommandBuffer;

    GridFrustumsGenerator();
    GridFrustumsGenerator(const GridFrustumsGenerator&) = delete;
    GridFrustumsGenerator(GridFrustumsGenerator&&) = delete;
    GridFrustumsGenerator& operator=(const GridFrustumsGenerator&) = delete;
    GridFrustumsGenerator& operator=(GridFrustumsGenerator&&) = delete;
    ~GridFrustumsGenerator();


public:
    static GridFrustumsGenerator& Instance();

    bool Init(const DevicePtr& device);
    bool Generate(const GridFrustumsGenerationDesc& desc, BufferPtr& frustumData);
};

} // namespace Renderer
} // namespace ABench
