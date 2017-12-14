#pragma once

#include "Renderer/LowLevel/Buffer.hpp"
#include "Renderer/LowLevel/MultiPipeline.hpp"
#include "Renderer/LowLevel/DescriptorAllocator.hpp"
#include "Renderer/LowLevel/CommandBuffer.hpp"
#include "Renderer/LowLevel/Tools.hpp"
#include "Renderer/LowLevel/Device.hpp"
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
    VkRAII<VkDescriptorSetLayout> mGridFrustumsDataSetLayout;
    VkRAII<VkPipelineLayout> mPipelineLayout;
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
    BufferPtr Generate(const GridFrustumsGenerationDesc& desc);
};

} // namespace Renderer
} // namespace ABench
