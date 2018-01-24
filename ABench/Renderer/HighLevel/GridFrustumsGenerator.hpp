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
    Buffer mGridFrustumsData;
    VkDescriptorSet mGridFrustumsDataSet;
    VkRAII<VkDescriptorSetLayout> mGridFrustumsDataSetLayout;
    VkRAII<VkPipelineLayout> mPipelineLayout;
    MultiPipeline mPipeline;
    CommandBuffer mDispatchCommandBuffer;

public:
    GridFrustumsGenerator();

    bool Init(const DevicePtr& device);
    bool Generate(const GridFrustumsGenerationDesc& desc);

    ABENCH_INLINE const Buffer* GetGridFrustums() const
    {
        return &mGridFrustumsData;
    }
};

} // namespace Renderer
} // namespace ABench
