#pragma once

#include "Buffer.hpp"
#include "MultiPipeline.hpp"


namespace ABench {
namespace Renderer {

struct GridFrustumsGeneratorDesc
{
    uint32_t viewportWidth;
    uint32_t viewportHeight;
};

class GridFrustumsGenerator
{
    uint32_t mFrustumsPerWidth;
    uint32_t mFrustumsPerHeight;
    BufferDesc mGridFrustumsDataDesc;
    Buffer mGridFrustumsData;
    VkPipelineLayout mPipelineLayout;
    MultiPipeline mPipeline;

public:
    GridFrustumsGenerator();
    ~GridFrustumsGenerator();

    bool Init(const GridFrustumsGeneratorDesc& desc);
    bool Generate(uint32_t viewportWidth, uint32_t viewportHeight);
};

} // namespace Renderer
} // namespace ABench
